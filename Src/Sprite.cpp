/**
* @file Sprite.cpp
*/
#include "Sprite.h"
#include "GLContext.h"
#include "GameData.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>
#include <limits>

/**
* �R���X�g���N�^.
*
* @param pos    �\������ʒu.
* @param tex    �`��ɏ̂���e�N�X�`��.
* @param origin �\���͈͂̍������W(�e�N�X�`�����W�n).
* @param size   �\���͈͂̑傫��(�e�N�X�`�����W�n).
*/
Sprite::Sprite(const glm::vec3& pos, std::shared_ptr<Texture::Image2D> tex,
  const glm::vec2& origin, const glm::vec2& size) :
  texture(tex), imageOrigin(origin), imageSize(size), position(pos)
{
}

/**
* �X�v���C�g�̏�Ԃ��X�V����.
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b).
*/
void Sprite::Update(float deltaTime)
{
  // �����`�F�b�N.
  if (lifespan > 0) {
    // ���������炵�����ʂ�0�ȉ��ɂȂ����玀�S.
    lifespan -= deltaTime;
    if (lifespan <= 0) {
      isDead = true;
    }
  }

  // �ړ����x�ɏd�͂����Z.
  const glm::vec3 gravity = GameData::Get().gravity;
  velocity += gravity * gravityScale * deltaTime;

  // ���W���X�V.
  position += velocity * deltaTime;
  rotation += angularVelocity * deltaTime;
  scale += scaleVelocity * deltaTime;
  color += colorVelocity * deltaTime;
}

/**
* �X�v���C�g�̔z����X�V����.
*
* @param sprites   �X�v���C�g�̔z��
* @param deltaTime �O��̍X�V����̌o�ߎ���.
*/
void UpdateSpriteList(std::vector<std::shared_ptr<Sprite>>& sprites, float deltaTime)
{
  // �z�񂪋�Ȃ牽�����Ȃ�.
  if (sprites.empty()) {
    return;
  }

  // ���ׂẴX�v���C�g�̎����A���W�A���x�Ȃǂ��X�V.
  for (auto& e : sprites) {
    e->Update(deltaTime);
  }

  // dead��Ԃ̃X�v���C�g���폜.
  const auto isDead = [](std::shared_ptr<Sprite> p) { return p->isDead; };
  const auto i = std::remove_if(sprites.begin(), sprites.end(), isDead);
  sprites.erase(i, sprites.end());
}

/**
* �f�X�g���N�^.
*/
SpriteRenderer::~SpriteRenderer()
{
  Free();
}

/**
* �X�v���C�g�p�̃��������m�ۂ���.
*
* @param maxSpriteCount �i�[�\�ȍő�X�v���C�g��.
*
* @retval true  �m�ې���.
* @retval false �m�ێ��s�A�܂��͊��Ɋm�ۍς�.
*/
bool SpriteRenderer::Allocate(size_t maxSpriteCount)
{
  // vao�����݂���ꍇ�͍쐬�ς�.
  if (buffers[1].vao) {
    std::cerr << "[�x��]" << __func__ << ": VAO�͍쐬�ς݂ł�.\n";
    return false;
  }

  // �C���f�b�N�X�f�[�^���쐬.
  const size_t maxIndex = std::min<size_t>(maxSpriteCount * 6, 65536) / 6;
  std::vector<GLushort> indices;
  indices.resize(maxIndex * 6);
  for (GLushort i = 0; i < maxIndex; ++i) {
    const GLushort vertexIndex = i * 4;
    const size_t arrayIndex = static_cast<size_t>(i) * 6;
    indices[arrayIndex + 0] = vertexIndex + 0;
    indices[arrayIndex + 1] = vertexIndex + 1;
    indices[arrayIndex + 2] = vertexIndex + 2;
    indices[arrayIndex + 3] = vertexIndex + 2;
    indices[arrayIndex + 4] = vertexIndex + 3;
    indices[arrayIndex + 5] = vertexIndex + 0;
  }
  ibo = GLContext::CreateBuffer(indices.size() * sizeof(GLushort), indices.data());

  // GPU���������m�ۂ��AVAO���쐬.
  const GLbitfield flags = GL_DYNAMIC_STORAGE_BIT;// GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
  // �ő�X�v���C�g����4�{���K�v�Ȓ��_��.
  const GLsizei vertexCount = static_cast<GLsizei>(maxSpriteCount * 4);
  for (auto& e : buffers) {
    e.vboPosition = GLContext::CreateBuffer(vertexCount * sizeof(glm::vec3), nullptr, flags);
    e.vboColor = GLContext::CreateBuffer(vertexCount * sizeof(glm::vec4), nullptr, flags);
    e.vboTexcoord = GLContext::CreateBuffer(vertexCount * sizeof(glm::vec2), nullptr, flags);
    e.vboNormal = GLContext::CreateBuffer(vertexCount * sizeof(glm::vec3), nullptr, flags);
    e.vao = GLContext::CreateVertexArray(e.vboPosition, e.vboColor, e.vboTexcoord, e.vboNormal, ibo);
    if (!e.vboPosition || !e.vboColor || !e.vboTexcoord || !e.vboNormal || !ibo || !e.vao) {
      std::cerr << "[�G���[]" << __func__ << ": VAO�̍쐬�Ɏ��s.\n";
      Free();
      return false;
    }
  }

  primitives.reserve(100);
  this->maxSpriteCount = maxSpriteCount;

  return true;
}

/**
* �`��f�[�^��j����GPU���������������.
*/
void SpriteRenderer::Free()
{
  primitives.clear();
  maxSpriteCount = 0;
  updatingBufferIndex = 0;

  for (auto& e : buffers) {
    glDeleteVertexArrays(1, &e.vao);
    e.vao = 0;
    glDeleteBuffers(1, &e.vboTexcoord);
    e.vboTexcoord = 0;
    glDeleteBuffers(1, &e.vboColor);
    e.vboColor = 0;
    glDeleteBuffers(1, &e.vboPosition);
    e.vboPosition = 0;
    glDeleteBuffers(1, &e.vboNormal);
    e.vboNormal = 0;
  }
  glDeleteBuffers(1, &ibo);
  ibo = 0;
}

/**
* �`��f�[�^���X�V����.
*
* @param sprites  �`�悷��X�v���C�g�̔z��.
* @param matView  �`��Ɏg�p����r���[�s��.
*/
void SpriteRenderer::Update(const std::vector<std::shared_ptr<Sprite>>& sprites, const glm::mat4& matView)
{
  // �`��f�[�^���폜.
  primitives.clear();

  // �X�v���C�g���ЂƂ��Ȃ���΂���ȏ��邱�Ƃ͂Ȃ�.
  if (sprites.empty()) {
    return;
  }

  // �X�v���C�g���J��������̋������ŕ��ׂ��z��tmp���쐬.
  using SortingData = std::pair<float, const Sprite*>;
  std::vector<SortingData> tmp;
  tmp.resize(sprites.size());
  for (size_t i = 0; i < sprites.size(); ++i) {
    tmp[i].first = (matView * glm::vec4(sprites[i]->position, 1)).z;
    tmp[i].second = sprites[i].get();
  }
  std::sort(tmp.begin(), tmp.end(),
    [](const SortingData& a, const SortingData& b) {
      return a.first < b.first;
    });

  // �\���v�����ꂽ�X�v���C�g�̐����������āA�m�ۂ���GPU�������ł͑���Ȃ��ꍇ�A
  // �x�����b�Z�[�W��\�����A����؂�Ȃ����͐؂�̂Ă�.
  if (tmp.size() > maxSpriteCount) {
    std::cout << "[�x��]" << __func__ <<
      ": �X�v���C�g�����������܂�(�v��=" << tmp.size() <<
      "/�ő�=" << maxSpriteCount << ").\n";
    tmp.resize(maxSpriteCount);
  }

  // �X�v���C�g���J�����Ɍ�����u�t�r���[��]�s��v���쐬����.
  // 1. ���s�ړ��������������邽��glm::mat3�R���X�g���N�^�ō���3x3���擾.
  // 2. �g��k���������������邽��inverse-transpose�ϊ����s��.
  // 3. �J�����̉�]��ł��������߁A��]�����̋t�s����쐬.
  const glm::mat3 matViewR = glm::transpose(glm::inverse(glm::mat3(matView)));
  const glm::mat4 matInverseViewR = glm::inverse(matViewR);

  // ���_�f�[�^���i�[����z���p��.
  // GPU�������ɃR�s�[����������s�v�Ȃ̂Ń��[�J���ϐ����g��.
  std::vector<glm::vec3> positions;
  std::vector<glm::vec4> colors;
  std::vector<glm::vec2> texcoords;
  std::vector<glm::vec3> normals;
  const size_t vertexCount = tmp.size() * 4;
  positions.resize(vertexCount);
  colors.resize(vertexCount);
  texcoords.resize(vertexCount);
  normals.resize(vertexCount);

  // �ŏ��̃v���~�e�B�u��ݒ�.
  primitives.push_back({ 0, 0, tmp[0].second->texture });

  // ���ׂẴX�v���C�g�𒸓_�f�[�^�ɕϊ�.
  for (size_t i = 0; i < tmp.size(); ++i) {
    const Sprite& sprite = *tmp[i].second;

    // �e�N�X�`�����W�̉E��origin�Ƒ傫��size���擾.
    const glm::vec2 origin = sprite.imageOrigin;
    const glm::vec2 size = sprite.imageSize;

    // �@���p�ƒ��_�p�̍��W�ϊ��s����쐬.
    const glm::mat4 matT = glm::translate(glm::mat4(1), sprite.position);
    const glm::mat4 matR = glm::rotate(glm::mat4(1), sprite.rotation, glm::vec3(0, 0, 1));
    const glm::mat4 matS = glm::scale(glm::mat4(1), glm::vec3(sprite.scale, 1));
    const glm::mat3 matNormal = matInverseViewR * matR;
    const glm::mat4 matModel = matT * matInverseViewR * matR * matS;

    // �f�[�^�̊i�[�J�n�ʒuv���v�Z.
    size_t v = i * 4;

    // ���W�A�F�A�e�N�X�`�����W�A�@����K�v�ɉ����ĕϊ����Ĕz��ɑ��.
    positions[v] = matModel * glm::vec4(-0.5f, -0.5f, 0, 1);
    colors[v] = sprite.color;
    texcoords[v] = origin;
    normals[v] = matNormal * glm::normalize(glm::vec3(-1, -1, 1));
    ++v; // ���̊i�[�ʒu��.

    positions[v] = matModel * glm::vec4(0.5f, -0.5f, 0, 1);
    colors[v] = sprite.color;
    texcoords[v] = glm::vec2(origin.x + size.x, origin.y);
    normals[v] = matNormal * glm::normalize(glm::vec3(1, -1, 1));
    ++v; // ���̊i�[�ʒu��.

    positions[v] = matModel * glm::vec4(0.5f, 0.5f, 0, 1);
    colors[v] = sprite.color;
    texcoords[v] = origin + size;
    normals[v] = matNormal * glm::normalize(glm::vec3(1, 1, 1));
    ++v; // ���̊i�[�ʒu��.

    positions[v] = matModel * glm::vec4(-0.5f, 0.5f, 0, 1);
    colors[v] = sprite.color;
    texcoords[v] = glm::vec2(origin.x, origin.y + size.y);
    normals[v] = matNormal * glm::normalize(glm::vec3(-1, 1, 1));

    // �`��v���~�e�B�u���X�V.
    Primitive& e = primitives.back();
    if (e.texture == sprite.texture) {
      // �����e�N�X�`�����g���X�v���C�g�̏ꍇ�̓f�[�^���𑝂₷����.
      // �������A�C���f�b�N�X���ő�l�𒴂���ꍇ�͐V�����`��f�[�^��ǉ�.
      if (e.count + 6 <= 65536) {
        e.count += 6;
      } else {
        const GLint vertexCount = (e.count / 6) * 4;
        primitives.push_back({ 6, e.baseVertex + vertexCount, sprite.texture });
      }
    } else {
      // �e�N�X�`��������Ă���ꍇ�͐V�����`��f�[�^��ǉ�.
      const GLint vertexCount = (e.count / 6) * 4;
      primitives.push_back({ 6, e.baseVertex + vertexCount, sprite.texture });
    }
  }

  // ���_�f�[�^��GPU�������ɃR�s�[.
  Buffer& e = buffers[updatingBufferIndex];
  glNamedBufferSubData(e.vboPosition, 0, positions.size() * sizeof(glm::vec3), positions.data());
  glNamedBufferSubData(e.vboColor, 0, colors.size() * sizeof(glm::vec4), colors.data());
  glNamedBufferSubData(e.vboTexcoord, 0, texcoords.size() * sizeof(glm::vec2), texcoords.data());
  glNamedBufferSubData(e.vboNormal, 0, normals.size() * sizeof(glm::vec3), normals.data());

  // �R�s�[���GPU��������؂�ւ���.
  updatingBufferIndex = !updatingBufferIndex;
}

/**
* �X�v���C�g��`�悷��.
*
* @param pipeline �`��Ɏg�p����O���t�B�b�N�X�p�C�v���C��.
* @param matVP    �`��Ɏg�p����r���[�v���W�F�N�V�����s��.
*/
void SpriteRenderer::Draw(
  std::shared_ptr<Shader::Pipeline> pipeline,
  const glm::mat4& matVP) const
{
  // �f�[�^���Ȃ���Ή������Ȃ�.
  if (primitives.empty()) {
    return;
  }

  // �p�C�v���C�����o�C���h���A�e��f�[�^��ݒ肷��.
  pipeline->Bind();
  pipeline->SetMVP(matVP);
  pipeline->SetModelMatrix(glm::mat4(1));
  pipeline->SetObjectColor(glm::vec4(1));

  // �[�x�e�X�g�͍s�����A�������݂͂��Ȃ��悤�ɐݒ�.
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // �A���t�@�u�����f�B���O��L����.
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // ���ʂ��`�悷��悤�ɐݒ�.
  glDisable(GL_CULL_FACE);

  // VAO���o�C���h.
  glBindVertexArray(buffers[!updatingBufferIndex].vao);

  // �`��f�[�^�����Ԃɕ`�悷��.
  for (const auto& e : primitives) {
    e.texture->Bind(0);
    glDrawElementsBaseVertex(GL_TRIANGLES, e.count, GL_UNSIGNED_SHORT, nullptr, e.baseVertex);
  }

  // �e�N�X�`���̃o�C���h������.
  const GLuint id = 0;
  glBindTextures(0, 1, &id);

  // VAO�̃o�C���h������.
  glBindVertexArray(0);

  // �[�x�o�b�t�@�ւ̏������݂�����.
  glDepthMask(GL_TRUE);

  // ���ʂ͕`�悵�Ȃ��悤�ɐݒ�.
  glEnable(GL_CULL_FACE);
}

