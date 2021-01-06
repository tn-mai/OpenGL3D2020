/**
* @file Mesh.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "Mesh.h"
#include "GLContext.h"
#include <glm/glm.hpp>
#include <fstream>
#include <string>
#include <unordered_map>
#include <stdio.h>
#include <iostream>

/**
* �}�`�f�[�^�Ɋւ��閼�O���.
*/
namespace Mesh {

/**
* �f�[�^���o�b�t�@�I�u�W�F�N�g�ɃR�s�[����.
*
* @param id          �R�s�[��ƂȂ�o�b�t�@�I�u�W�F�N�gID.
* @param unitSize    �v�f�̃o�C�g��.
* @param offsetCount �R�s�[��I�t�Z�b�g(�v�f�P��).
* @param count       �R�s�[����v�f��.
* @param data        �R�s�[����f�[�^�̃A�h���X.
*
* @retval true  �R�s�[����.
* @retval false �R�s�[���s.
*/
bool CopyData(GLuint id, size_t unitSize, GLsizei offsetCount, size_t count, const void* data)
{
  const GLsizei size = static_cast<GLsizei>(count * unitSize);
  const GLuint tmp = GLContext::CreateBuffer(size, data);
  if (!tmp) {
    std::cerr << "[�G���[]" << __func__ << ": �R�s�[���o�b�t�@�̍쐬�Ɏ��s(size=" << size << ").\n";
    return false;
  }
  const GLsizei offset = static_cast<GLsizei>(offsetCount * unitSize);
  glCopyNamedBufferSubData(tmp, id, 0, offset, size);
  glDeleteBuffers(1, &tmp);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ": �f�[�^�̃R�s�[�Ɏ��s(size=" << size << ", offset=" << offset << ").\n";
  }
  return true;
}

/**
* �v���~�e�B�u��`�悷��.
*
* @param morphTarget ���[�t�B���O�I�����̃v���~�e�B�u.
*                    ���[�t�B���O���Ȃ��ꍇ��nullptr���w�肷��.
*/
void Primitive::Draw(const Primitive* morphTarget) const
{
  primitiveBuffer->SetMorphBaseMesh(baseVertex);
  if (morphTarget) {
    primitiveBuffer->SetMorphTargetMesh(morphTarget->baseVertex);
  } else {
    primitiveBuffer->SetMorphTargetMesh(baseVertex);
  }
  glDrawElementsBaseVertex(mode, count, GL_UNSIGNED_SHORT, indices, 0);
}

/**
* �f�X�g���N�^.
*/
PrimitiveBuffer::~PrimitiveBuffer()
{
  Free();
}

/**
* �v���~�e�B�u�p�̃��������m�ۂ���.
*
* @param maxVertexCount  �i�[�\�ȍő咸�_��.
* @param maxIndexCount   �i�[�\�ȍő�C���f�b�N�X��.
*
* @retval true  �m�ې���.
* @retval false �m�ێ��s�A�܂��͊��Ɋm�ۍς�.
*/
bool PrimitiveBuffer::Allocate(GLsizei maxVertexCount, GLsizei maxIndexCount)
{
  if (vao) {
    std::cerr << "[�x��]" << __func__ << ": VAO�͍쐬�ς݂ł�.\n";
    return false;
  }
  vboPosition = GLContext::CreateBuffer(sizeof(glm::vec3) * maxVertexCount, nullptr);
  vboColor = GLContext::CreateBuffer(sizeof(glm::vec4) * maxVertexCount, nullptr);
  vboTexcoord = GLContext::CreateBuffer(sizeof(glm::vec2) * maxVertexCount, nullptr);
  vboNormal = GLContext::CreateBuffer(sizeof(glm::vec3) * maxVertexCount, nullptr);
  ibo = GLContext::CreateBuffer(sizeof(GLushort) * maxIndexCount, nullptr);
  vao = GLContext::CreateVertexArray(vboPosition, vboColor, vboTexcoord, vboNormal, ibo);
  if (!vboPosition || !vboColor || !vboTexcoord || !vboNormal || !ibo || !vao) {
    std::cerr << "[�G���[]" << __func__ << ": VAO�̍쐬�Ɏ��s.\n";
    Free();
    return false;
  }
  primitives.reserve(100);
  this->maxVertexCount = maxVertexCount;
  this->maxIndexCount = maxIndexCount;
  return true;
}

/**
* �v���~�e�B�u�p�̃��������������.
*/
void PrimitiveBuffer::Free()
{
  primitives.clear();

  glDeleteVertexArrays(1, &vao);
  vao = 0;
  glDeleteBuffers(1, &ibo);
  ibo = 0;
  glDeleteBuffers(1, &vboTexcoord);
  vboTexcoord = 0;
  glDeleteBuffers(1, &vboColor);
  vboColor = 0;
  glDeleteBuffers(1, &vboPosition);
  vboPosition = 0;

  maxVertexCount = 0;
  curVertexCount = 0;
  maxIndexCount = 0;
  curIndexCount = 0;
}

/**
* �v���~�e�B�u��ǉ�����.
*
* @param vertexCount �ǉ����钸�_�f�[�^�̐�.
* @param pPosition   ���W�f�[�^�ւ̃|�C���^.
* @param pColor      �F�f�[�^�ւ̃|�C���^.
* @param pTexcoord   �e�N�X�`�����W�f�[�^�ւ̃|�C���^.
* @param pNormal     �@���f�[�^�ւ̃|�C���^.
* @param indexCount  �ǉ�����C���f�b�N�X�f�[�^�̐�.
* @param pIndex      �C���f�b�N�X�f�[�^�ւ̃|�C���^.
*
* @retval true  �ǉ�����.
* @retval false �ǉ����s.
*/
bool PrimitiveBuffer::Add(size_t vertexCount, const glm::vec3* pPosition,
  const glm::vec4* pColor, const glm::vec2* pTexcoord, const glm::vec3* pNormal, size_t indexCount, const GLushort* pIndex)
{
  if (!vao) {
    std::cerr << "[�G���[]" << __func__ << ": VAO���쐬����Ă��܂���.\n";
    return false;
  } else if (maxVertexCount < curVertexCount) {
    std::cerr << "[�G���[]" << __func__ << ": ���_�J�E���g�Ɉُ킪����܂�(max=" <<
      maxVertexCount << ", cur=" << curVertexCount << ")\n";
    return false;
  } else if (maxIndexCount < curIndexCount) {
    std::cerr << "[�G���[]" << __func__ << ": �C���f�b�N�X�J�E���g�Ɉُ킪����܂�(max=" <<
      maxIndexCount << ", cur=" << curIndexCount << ")\n";
    return false;
  } else if (vertexCount > static_cast<size_t>(maxVertexCount) - curVertexCount) {
    std::cerr << "[�x��]" << __func__ << ": VBO�����t�ł�(max=" << maxVertexCount <<
      ", cur=" << curVertexCount << ", add=" << vertexCount << ")\n";
    return false;
  } else if (indexCount > static_cast<size_t>(maxIndexCount) - curIndexCount) {
    std::cerr << "[�x��]" << __func__ << ": IBO�����t�ł�(max=" << maxIndexCount <<
      ", cur=" << curIndexCount << ", add=" << indexCount << ")\n";
    return false;
  }

  if (!CopyData(vboPosition, sizeof(glm::vec3), curVertexCount, vertexCount, pPosition)) {
    return false;
  }
  if (!CopyData(vboColor, sizeof(glm::vec4), curVertexCount, vertexCount, pColor)) {
    return false;
  }
  if (!CopyData(vboTexcoord, sizeof(glm::vec2), curVertexCount, vertexCount, pTexcoord)) {
    return false;
  }
  if (!CopyData(vboNormal, sizeof(glm::vec3), curVertexCount, vertexCount, pNormal)) {
    return false;
  }
  if (!CopyData(ibo, sizeof(GLushort), curIndexCount, indexCount, pIndex)) {
    return false;
  }

  primitives.push_back(Primitive(GL_TRIANGLES, static_cast<GLsizei>(indexCount),
    sizeof(GLushort) * curIndexCount, curVertexCount, this));

  curVertexCount += static_cast<GLsizei>(vertexCount);
  curIndexCount += static_cast<GLsizei>(indexCount);

  return true;
}

/**
* �v���~�e�B�u��ǉ�����.
*
* @param filename ���[�h����OBJ�t�@�C����.
*
* @retval true  �ǉ�����.
* @retval false �ǉ����s.
*/
bool PrimitiveBuffer::AddFromObjFile(const char* filename)
{
  std::ifstream ifs(filename);
  if (!ifs) {
    std::cerr << "[�G���[]" << __func__ << ":`" << filename << "`���J���܂���.\n";
    return false;
  }

  // �f�[�^�ǂݎ��p�̕ϐ�������.
  std::vector<glm::vec3> positionList;
  std::vector<glm::vec2> texcoordList;
  std::vector<glm::vec3> normalList;
  struct Face {
    int v;
    int vt;
    int vn;

    bool operator==(const Face& o) const {
      return v == o.v && vt == o.vt && vn == o.vn;
    }
  };
  std::vector<Face> faceList;

  // �e�ʂ�\��.
  positionList.reserve(1000);
  texcoordList.reserve(1000);
  normalList.reserve(1000);
  faceList.reserve(1000);

  // �t�@�C�����烂�f���̃f�[�^��ǂݍ���.
  size_t lineNo = 0; // �ǂݍ��񂾍s��.
  while (!ifs.eof()) {
    std::string line;
    std::getline(ifs, line);
    ++lineNo;

    // ��s�͖���.
    if (line.empty()) {
      continue;
    }

    const std::string type = line.substr(0, line.find(' '));

    // �R�����g�s�͖���.
    if (type == "#") {
      continue;
    }

    const char* p = line.c_str() + type.size();
    if (type == "v") { // ���_���W.
      glm::vec3 v(0);
      if (sscanf(p, "%f %f %f", &v.x, &v.y, &v.z) != 3) {
        std::cerr << "[�G���[]" << __func__ << ":���_���W�̓ǂݎ��Ɏ��s.\n" <<
          "  " << filename << "(" << lineNo << "�s��): " << line << "\n";
      }
      positionList.push_back(v);
    } else if (type == "vt") { // �e�N�X�`�����W.
      glm::vec2 vt(0);
      if (sscanf(p, "%f %f", &vt.x, &vt.y) != 2) {
        std::cerr << "[�G���[]" << __func__ << ":�e�N�X�`�����W�̓ǂݎ��Ɏ��s.\n" <<
          "  " << filename << "(" << lineNo << "�s��): " << line << "\n";
      }
      texcoordList.push_back(vt);
    } else if (type == "vn") { // �@��.
      glm::vec3 vn(0);
      if (sscanf(p, "%f %f %f", &vn.x, &vn.y, &vn.z) != 3) {
        std::cerr << "[�G���[]" << __func__ << ":�@���̓ǂݎ��Ɏ��s.\n" <<
          "  " << filename << "(" << lineNo << "�s��): " << line << "\n";
      }
      // �@���𐳋K��.
      if (glm::dot(vn, vn) > 0) {
        vn = glm::normalize(vn);
      } else {
        vn = glm::vec3(0, 1, 0);
      }
      normalList.push_back(vn);
    } else if (type == "f") { // ��.
      // �O�p�`�Ǝl�p�`�̂ݑΉ�.
      Face f[4];
      const int n = sscanf(p, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
        &f[0].v, &f[0].vt, &f[0].vn,
        &f[1].v, &f[1].vt, &f[1].vn,
        &f[2].v, &f[2].vt, &f[2].vn,
        &f[3].v, &f[3].vt, &f[3].vn);
      // �C���f�b�N�X�������̏ꍇ�A���݂̃f�[�^�ʒu����̑��΃I�t�Z�b�g�Ƃ��Ĉ���.
      for (int i = 0; i < n / 3; ++i) {
        if (f[i].v < 0) {
          f[i].v += static_cast<int>(positionList.size());
        }
        if (f[i].vt < 0) {
          f[i].vt += static_cast<int>(texcoordList.size());
        }
        if (f[i].vn < 0) {
          f[i].vn += static_cast<int>(normalList.size());
        }
      }

      // �f�[�^����9�̂Ƃ��͎O�p�`�A12�̂Ƃ��͎l�p�`.
      if (n == 9) {
        for (int i = 0; i < 3; ++i) {
          faceList.push_back(f[i]);
        }
      } else if (n == 12) {
        static const int indices[] = { 0, 1, 2, 2, 3, 0 };
        for (int i = 0; i < 6; ++i) {
          faceList.push_back(f[indices[i]]);
        }
      } else {
        std::cerr << "[�x��]" << __func__ << ":�ʂ̒��_����3�܂���4�łȂ��Ă͂Ȃ�܂���(���_��=" << n << "). " << filename << "(" << lineNo << "�s��).\n";
      }
    } else {
      std::cerr << "[�x��]" << __func__ << ":���Ή��̌`���ł�.\n" <<
        "  " << filename << "(" << lineNo << "�s��): " << line << "\n";
    }
  }

  // ���_�f�[�^�ƃC���f�b�N�X�f�[�^�p�̕ϐ�������.
  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> texcoords;
  std::vector<glm::vec3> normals;
  std::vector<GLushort> indices;

  const size_t indexCount = faceList.size();
  positions.reserve(indexCount / 3);
  texcoords.reserve(indexCount / 3);
  normals.reserve(indexCount / 3);
  indices.reserve(indexCount);

  // ���L���_�����p�̘A�z�z��.
  struct FaceHash {
    size_t operator()(const Face& f) const {
      const std::hash<int> hash;
      return hash(f.v) ^ hash(f.vt << 10) ^ hash(f.vn << 20);
    }
  };
  std::unordered_map<Face, GLushort, FaceHash> shareableVertexMap;

  // ���f���̃f�[�^�𒸓_�f�[�^�ƃC���f�b�N�X�f�[�^�ɕϊ�����.
  for (size_t i = 0; i < indexCount; ++i) {
    // ���L�\�ȃf�[�^������.
    const auto itr = shareableVertexMap.find(faceList[i]);
    if (itr != shareableVertexMap.end()) {
      // ���L�\�Ȗʃf�[�^������������A���̖ʃf�[�^�̃C���f�b�N�X���g��.
      indices.push_back(itr->second);
      continue;
    }

    // ���L�\�Ȗʃf�[�^��������Ȃ���΁A�V�����C���f�b�N�X�ƒ��_�f�[�^���쐬����.
    indices.push_back(static_cast<GLushort>(positions.size()));

    // �V�����ʃf�[�^�ƃC���f�b�N�X�����L�\�Ȗʃf�[�^�Ƃ��Ēǉ�.
    shareableVertexMap.emplace(faceList[i], static_cast<GLushort>(positions.size()));
  
    // ���_���W��ϊ�.
    const int v = faceList[i].v - 1;
    if (v < static_cast<int>(positionList.size())) {
      positions.push_back(positionList[v]);
    } else {
      std::cerr << "[�x��]" << __func__ << ":���_���W�C���f�b�N�X" << v <<
        "�͔͈�[0, " << positionList.size() - 1 << "]�̊O���w���Ă��܂�.\n" <<
        "  " << filename << "\n";
      positions.push_back(glm::vec3(0));
    }

    // �e�N�X�`�����W��ϊ�.
    const int vt = faceList[i].vt - 1;
    if (vt < static_cast<int>(texcoordList.size())) {
      texcoords.push_back(texcoordList[vt]);
    } else {
      std::cerr << "[�x��]" << __func__ << ":�e�N�X�`�����W�C���f�b�N�X" << vt <<
        "�͔͈�[0, " << texcoordList.size() - 1 << "]�̊O���w���Ă��܂�.\n" <<
        "  " << filename << "\n";
      texcoords.push_back(glm::vec2(0));
    }

    // �@����ϊ�.
    const int vn = faceList[i].vn - 1;
    if (vn < static_cast<int>(normalList.size())) {
      normals.push_back(normalList[vn]);
    } else {
      std::cerr << "[�x��]" << __func__ << ":�@���C���f�b�N�X" << vn <<
        "�͔͈�[0, " <<normalList.size() - 1 << "]�̊O���w���Ă��܂�.\n" <<
        "  " << filename << "\n";
      normals.push_back(glm::vec3(0, 1, 0));
    }
  }

  // �v���~�e�B�u��ǉ�����.
  const std::vector<glm::vec4> colors(positions.size(), glm::vec4(1));
  const bool result = Add(positions.size(), positions.data(), colors.data(),
    texcoords.data(), normals.data(), indices.size(), indices.data());
  if (result) {
    std::cout << "[���]" << __func__ << ":" << filename << "(���_��=" <<
      positions.size() << " �C���f�b�N�X��=" << indices.size() << ")\n";
  } else {
    std::cerr << "[�x��]" << __func__ << ":" << filename << "�̓ǂݍ��݂Ɏ��s.\n";
  }
  return result;
}

/**
* �v���~�e�B�u���擾����.
*
* @param n �v���~�e�B�u�̃C���f�b�N�X.
*
* @return n�ɑΉ�����v���~�e�B�u.
*/
const Primitive& PrimitiveBuffer::Get(size_t n) const
{
  if (n < 0 || n > static_cast<int>(primitives.size())) {
    std::cerr << "[�x��]" << __func__ << ":" << n <<
      "�͖����ȃC���f�b�N�X�ł�(�L���͈�0�`" << primitives.size() - 1 << ").\n";
    static const Primitive dummy;
    return dummy;
  }
  return primitives[n];
}

/**
* VAO���o�C���h����.
*/
void PrimitiveBuffer::BindVertexArray() const
{
  glBindVertexArray(vao);
}

/**
* VAO�̃o�C���h����������.
*/
void PrimitiveBuffer::UnbindVertexArray() const
{
  glBindVertexArray(0);
}

/**
* ���[�t�B���O�J�n���̃��b�V����ݒ肷��.
*
* @param baseVertex ���_�f�[�^�̈ʒu.
*/
void PrimitiveBuffer::SetMorphBaseMesh(GLuint baseVertex) const
{
  GLContext::SetMorphBaseMesh(vao, vboPosition, vboColor, vboTexcoord, vboNormal, baseVertex);
}

/**
* ���[�t�B���O�I�����̃��b�V����ݒ肷��.
*
* @param baseVertex ���_�f�[�^�̈ʒu.
*/
void PrimitiveBuffer::SetMorphTargetMesh(GLuint baseVertex) const
{
  GLContext::SetMorphTargetMesh(vao, vboPosition, vboNormal, baseVertex);
}

} // namespace Mesh

