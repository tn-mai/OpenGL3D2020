/**
* @file Actor.cpp
*/
#include "Actor.h"
#include "GameData.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>

/// ���[�t�B���O�A�j���[�V�����̐؂�ւ��ɂ����鎞��.
const float maxMorphTransitionTime = 0.2f;

/**
* �R���X�g���N�^.
*
* @param name �쐬����A�N�^�[�ɕt���閼�O.
* @param pid       �A�N�^�[�̊O����\���v���~�e�B�u��ID.
* @param tex       �v���~�e�B�u�ɓ\��t����e�N�X�`��.
* @param pos       �A�N�^�[�̍��W.
*/
Actor::Actor(std::string name, const Mesh::Primitive* prim,
  std::shared_ptr<Texture::Image2D> tex, const glm::vec3& pos) :
  name(name), primitive(prim), texture(tex), position(pos)
{
}

/**
* �A�N�^�[�̏�Ԃ��X�V����.
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b).
*/
void Actor::Update(float deltaTime)
{
  OnUpdate(deltaTime);

  // �����`�F�b�N.
  if (lifespan > 0) {
    // ���������炵�����ʂ�0�ȉ��ɂȂ����玀�S.
    lifespan -= deltaTime;
    if (lifespan <= 0) {
      lifespan = 0;
      isDead = true;
    }
  }

  // ����ł�����X�V�I��.
  if (isDead) {
    return;
  }

  // �^�C�}�[���Z.
  if (timer > 0) {
    timer = std::max(0.0f, timer - deltaTime);
  }

  // �ړ����x�ɏd�͂����Z.
  if (gravityScale) {
    velocity += GameData::Get().gravity * gravityScale * deltaTime;
  }

  // ���C�ɂ�錸��.
  if (drag) {
    const glm::vec3 v(velocity.x, 0, velocity.z);
    const float speed = glm::length(v);
    if (speed > 0) {
      if (speed > -drag * deltaTime) {
        velocity += glm::normalize(v) * drag * deltaTime;
      } else {
        velocity.x = velocity.z = 0;
      }
    }
    drag = 0;
  }

  // ���W���X�V.
  position += velocity * deltaTime;

  // �Փ˂������Ȕ͈͂��X�V.
  const float vl = glm::length(velocity)* deltaTime * 2.0f;
  switch (collision.shape) {
  case Collision::Shape::cylinder:
    boundingBox.c = position;
    boundingBox.c.y += (collision.top + collision.bottom) * 0.5f;
    boundingBox.r.x = collision.radius + vl;
    boundingBox.r.y = (collision.top - collision.bottom) * 0.5f + vl;
    boundingBox.r.z = boundingBox.r.x;
    break;
  case Collision::Shape::box:
    boundingBox.c = position + (collision.boxMin + collision.boxMax) * 0.5f;
    boundingBox.r = (collision.boxMax - collision.boxMin) * 0.5f + vl;
    break;
  }

  // �A�j���[�V�����f�[�^������΃A�j���[�V��������.
  if (animation && !animation->list.empty()) {
    // ���[�v�t���O��true�A�܂��̓��[�v�t���O��false
    // ���A�j���[�V�����ԍ����v���~�e�B�u���X�g�̃f�[�^���𒴂��Ă��Ȃ��ꍇ�A
    // �A�j���[�V�������X�V����.
    if (animation->isLoop || animationNo < animation->list.size() - 1) {
      animationTimer += deltaTime;
      // �A�j���[�V�����^�C�}�[���C���^�[�o�����Ԃ𒴂��Ă�����A
      // �^�C�}�[�����炵�āA�A�j���[�V�����ԍ���E�߂�.
      if (animationTimer >= animation->interval) {
        animationTimer -= animation->interval;
        ++animationNo;
        // �A�j���[�V�����ԍ����v���~�e�B�u���X�g�̃f�[�^���𒴂����ꍇ�A
        // �A�j���[�V�����ԍ���0�ɖ߂�.
        if (animationNo >= animation->list.size()) {
          animationNo = 0;
        }
      }
    }
    primitive = animation->list[animationNo];

    // �A�j���[�V�����؂�ւ��^�C�}�[�����Z���A
    // 0�ȉ��ɂȂ�����؂�ւ������Ƃ��Ē��O�̃A�j���[�V����������.
    if (morphTransitionTimer > 0) {
      morphTransitionTimer -= deltaTime;
      if (morphTransitionTimer <= 0) {
        // ���O�̃A�j���[�V����������.
        prevMorphWeight = 0;
        prevBaseMesh = nullptr;
        prevMorphTarget = nullptr;
        morphTransitionTimer = 0;
      }
    }

    // ���[�t�^�[�Q�b�g���X�V.
    size_t nextAnimationNo = animationNo + 1;
    if (animation->isLoop) {
      nextAnimationNo %= animation->list.size();
    } else {
      nextAnimationNo = std::min(nextAnimationNo, animation->list.size() - 1);
    }
    morphTarget = animation->list[nextAnimationNo];
  }
}

/**
* �A�N�^�[��`�悷��.
*
* @param pipeline �s��̐ݒ��ƂȂ�p�C�v���C���I�u�W�F�N�g.
* @param matVP    �`��Ɏg�p����r���[�v���W�F�N�V�����s��.
* @param drawType �`��̎��.
*/
void Actor::Draw(const Shader::Pipeline& pipeline, const glm::mat4& matVP,
  DrawType drawType) const
{
  // �v���~�e�B�u���ݒ肳��Ă��Ȃ��Ƃ��͉��������I��.
  if (!primitive) {
    return;
  }

  // �e�`��̎��A�e�����Ȃ��A�N�^�[�͉��������I��.
  if (drawType == DrawType::shadow && !isShadowCaster) {
    return;
  }

  // ���s�ړ�������s������.
  const glm::mat4 matTranslate = glm::translate(glm::mat4(1), position);
  // X����]������s������.
  const glm::mat4 matRotateX = glm::rotate(
    glm::mat4(1), rotation.x, glm::vec3(1, 0, 0));
  // Y����]������s������.
  const glm::mat4 matRotateY = glm::rotate(
    glm::mat4(1), rotation.y, glm::vec3(0, 1, 0));
  // Z����]������s������.
  const glm::mat4 matRotateZ = glm::rotate(
    glm::mat4(1), rotation.z, glm::vec3(0, 0, 1));
  // �傫����ς���s������.
  const glm::mat4 matScale = glm::scale(glm::mat4(1), scale);

  // ���s�ړ��E��]�E�傫���ύX�̍s����|���Z���āA�ЂƂ̃��f���s��ɂ܂Ƃ߂�.
  const glm::mat4 matModel =
    matTranslate * matRotateY * matRotateZ * matRotateX * matScale;

  // GPU�������ɍs���]��.
  if (drawType == DrawType::color) {
    pipeline.SetObjectColor(baseColor);
    pipeline.SetModelMatrix(matModel);
  }
  pipeline.SetMVP(matVP * matModel);

  // �x�[�X���b�V���ƃ��[�t�^�[�Q�b�g�̍����䗦��ݒ�.
  if (animation) {
    pipeline.SetMorphWeight(glm::vec3(
      glm::clamp(animationTimer / animation->interval, 0.0f, 1.0f),
      prevMorphWeight,
      morphTransitionTimer / maxMorphTransitionTime));
  } else {
    pipeline.SetMorphWeight(glm::vec3(0));
  }

  const GameData& gamedata = GameData::Get();
  for (size_t i = 0; i < std::size(samplers); ++i) {
    if (samplers[i]) {
      samplers[i]->Bind(i);
    } else {
      gamedata.sampler.Bind(i);
    }
  }

  // �e�N�X�`���C���[�W�X���b�g0�ԂɃe�N�X�`�������蓖�Ă�.
  texture->Bind(0);
  if (texNormal) {
    texNormal->Bind(1);
  } else {
    Texture::UnbindTexture(1);
  }
  if (texMetallicSmoothness) {
    texMetallicSmoothness->Bind(2);
  } else {
    Texture::UnbindTexture(2);
  }

  // �v���~�e�B�u��`��.
  primitive->Draw(morphTarget, prevBaseMesh, prevMorphTarget);
}

/**
* �����~���̏Փ˔����ݒ肷��.
*
* @param top    �~���̏�[�̍��W.
* @param bottom �~���̉��[�̍��W.
* @param radius �~���̔��a.
*/
void Actor::SetCylinderCollision(float top, float bottom, float radius)
{
  collision.shape = Collision::Shape::cylinder;
  collision.top = top;
  collision.bottom = bottom;
  collision.radius = radius;

  boundingBox.c = position;
  boundingBox.c.y += (collision.top + collision.bottom) * 0.5f;
  boundingBox.r.x = collision.radius;
  boundingBox.r.y = (collision.top - collision.bottom) * 0.5f;
  boundingBox.r.z = boundingBox.r.x;
}

/**
* �����̂̏Փ˔����ݒ肷��.
*
* @param min �����̂̍ŏ����W.
* @param max �����̂̍ő���W.
*/
void Actor::SetBoxCollision(const glm::vec3& min, const glm::vec3& max)
{
  collision.shape = Collision::Shape::box;
  collision.boxMin = min;
  collision.boxMax = max;

  boundingBox.c = position + (collision.boxMin + collision.boxMax) * 0.5f;
  boundingBox.r = (collision.boxMax - collision.boxMin) * 0.5f;
}

/**
* �A�j���[�V������ݒ肷��.
*
* @param animation �A�j���[�V�����f�[�^.
*/
void Actor::SetAnimation(
  std::shared_ptr<Animation> animation)
{
  // ���ɓ����A�j���[�V�������ݒ肳��Ă���ꍇ�͉������Ȃ�.
  if (this->animation == animation) {
    return;
  }

  // ���݂̃A�j���[�V�����𒼑O�̃��[�t�B���O�f�[�^�Ƃ��ċL�^
  if (this->animation) {
    prevMorphWeight = glm::clamp(animationTimer / this->animation->interval, 0.0f, 1.0f);
    prevBaseMesh = primitive;
    prevMorphTarget = morphTarget;
    morphTransitionTimer = maxMorphTransitionTime;
  }

  this->animation = animation;
  animationNo = 0;
  animationTimer = 0;
  if (animation && !animation->list.empty()) {
    primitive = animation->list[0];
  }
}

/**
* �A�N�^�[���X�g���X�V����.
*
* @param actorList  �X�V����A�N�^�[���X�g.
* @param deltaTime  �O��̍X�V����̌o�ߎ���(�b).
*/
void UpdateActorList(ActorList& actorList, float deltaTime)
{
  // ��ԍX�V.
  for (size_t i = 0; i < actorList.size(); ++i) {
    actorList[i]->Update(deltaTime);
  }

  // dead��Ԃ̃A�N�^�[��OnDestroy���Ăяo��.
  for (auto& e : actorList) {
    if (e->isDead) {
      e->OnDestroy();
    }
  }

  // dead��Ԃ̃A�N�^�[���폜.
  const auto isDead = [](ActorPtr p) { return p->isDead; };
  const ActorList::iterator i = std::remove_if(actorList.begin(), actorList.end(), isDead);
  actorList.erase(i, actorList.end());
}

/**
* �A�N�^�[���X�g��`�悷��.
*
* @param actorList �`�悷��A�N�^�[���X�g.
* @param matVP     �`��Ɏg�p����r���[�v���W�F�N�V�����s��.
*/
void RenderActorList(const ActorList& actorList,
  const glm::mat4& matVP, Actor::DrawType drawType)
{
  GameData& global = GameData::Get();
  for (size_t i = 0; i < actorList.size(); ++i) {
    actorList[i]->Draw(*global.pipeline, matVP, drawType);
  }
}

/**
* �~���Ɖ~���̏Փ˂���������.
*
* @param a       �Փˌ`�󂪉~���̃A�N�^�[A.
* @param b       �Փˌ`�󂪉~���̃A�N�^�[B.
* @param isBlock �ђʂ����Ȃ��ꍇtrue. �ђʂ���ꍇfalse
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
*/
bool CollideCylinders(Actor& a, Actor& b, bool isBlock)
{
  // �~��A�̉��[���~��B�̏�[�̏�ɂ���Ȃ�Փ˂��Ă��Ȃ�.
  const float bottomA = a.position.y + a.collision.bottom;
  const float topB = b.position.y + b.collision.top;
  if (bottomA >= topB) {
    return false;
  }
  // �~��A�̏�[���~��B�̉��[�̉��ɂ���Ȃ�Փ˂��Ă��Ȃ�.
  const float topA = a.position.y + a.collision.top;
  const float bottomB = b.position.y + b.collision.bottom;
  if (topA <= bottomB) {
    return false;
  }
  // �A�N�^�[A��B�̏Փ˔���(�~��)�̒��S�Ԃ̋�����2��(d2)���v�Z.
  const float dx = a.position.x - b.position.x;
  const float dz = a.position.z - b.position.z;
  const float d2 = dx * dx + dz * dz;
  // �Փ˂��Ȃ�����r���v�Z.
  const float r = a.collision.radius + b.collision.radius;
  // d2���Փ˂��Ȃ�����r��2��ȏ�Ȃ�Փ˂��Ă��Ȃ�.
  if (d2 >= r * r) {
    return false;
  }

  // �u���b�N�w�肪�Ȃ���ΏՓ˂����Ƃ�����񂾂���Ԃ�.
  if (!isBlock) {
    return true;
  }

  // �u���b�N�w�肪����̂ő���������Ԃ�.

#if 1
  // �㉺�̉����Ԃ��������Z��������py�Ƃ���.
  float py = 0;
  const float y0 = bottomB - topA;
  const float y1 = topB - bottomA;
  if (std::abs(y0) <= std::abs(y1)) {
    py = y0;
  } else {
    py = y1;
  }

  // Y�����̉����o��������XZ�����̉����o���������Z�����Y�����ɉ����o��.
  // �������XZ�����ɉ����o��.
  // ���S�Ԃ̋���d���v�Z.
  const float d = std::sqrt(d2);
  // �����Ԃ�����s���v�Z.
  const float s = r - d;
  if (std::abs(py) < s) {
#if 0
    a.AddCollisionPenalty(glm::vec3(0, py * 0.5f, 0));
    b.AddCollisionPenalty(glm::vec3(0, -py * 0.5f, 0));
    if (py >= 0) {
      a.velocity.y = std::max(a.velocity.y, 0.0f);
    } else {
      b.velocity.y = std::max(b.velocity.y, 0.0f);
    }
    a.drag = (a.friction + b.friction) * 0.5f * GameData::Get().gravity.y;
#else
    // ��������߂��ꍇ��A����Ɉړ�.
    if (py >= 0) {
      a.position.y += topB - bottomA; // A����Ɉړ�.
      a.velocity.y = std::max(a.velocity.y, 0.0f);
      a.drag = (a.friction + b.friction) * 0.5f * GameData::Get().gravity.y;
    } else {
      b.position.y += topA - bottomB; // B����Ɉړ�.
      b.velocity.y = std::max(b.velocity.y, 0.0f);
      b.drag = (a.friction + b.friction) * 0.5f * GameData::Get().gravity.y;
    }
#endif
  } else {
    // A��B���҂�����d�Ȃ��Ă���ꍇ��+X�����ɉ����o��.
    if (d2 < 0.00001f) {
      // �A�N�^�[A��B���ϓ��ɉ����Ԃ�.
      const glm::vec3 n(1, 0, 0);
      a.position += n * r * 0.5f;
      b.position -= n * r * 0.5f;
    } else {
      // �~���̒��S���Ԃ̕����x�N�g��n���v�Z.
      const glm::vec3 n(dx / d, 0, dz / d);
      // �A�N�^�[A��B���ϓ��ɉ����Ԃ�.
      a.position += n * s * 0.5f;
      b.position -= n * s * 0.5f;
    }
  }
#else
  // Y�������̏d�Ȃ��Ă��镔���̒������v�Z.
  const float overlapY = std::min(topA, topB) - std::max(bottomA, bottomB);

  // �Z���ق��̉~���̍����̔������v�Z.
  const float shortY = std::min(topA - bottomA, topB - bottomB);

  // �d�Ȃ��Ă��钷�����Z�����̉~���̍����̔��������Ȃ��ɉ����Ԃ�.
  // �����ȏ�Ȃ牡�ɉ����Ԃ�.
  if (overlapY < shortY * 0.75f) {
    // ���[�������ʒu�ɂ���A�N�^�[����Ɉړ�.
    if (bottomA > bottomB) {
      a.position.y += topB - bottomA; // A����Ɉړ�.
      a.velocity.y = std::max(a.velocity.y, 0.0f);
      a.drag = (a.friction + b.friction) * 0.5f * GameData::Get().gravity.y;
    } else {
      b.position.y += topA - bottomB; // B����Ɉړ�.
      b.velocity.y = std::max(b.velocity.y, 0.0f);
      b.drag = (a.friction + b.friction) * 0.5f * GameData::Get().gravity.y;
    }
  } else if (d2 < 0.0001f) {
    // �A�N�^�[A��B���ϓ��ɉ����Ԃ�.
    const glm::vec3 n(1, 0, 0);
    a.position += n * r * 0.5f;
    b.position -= n * r * 0.5f;
  } else {
    // ���S�Ԃ̋���d���v�Z.
    const float d = std::sqrt(d2);
    // �����Ԃ�����s���v�Z.
    const float s = r - d;
    // �~���̒��S���Ԃ̕����x�N�g��n���v�Z.
    const glm::vec3 n(dx / d, 0, dz / d);
    // �A�N�^�[A��B���ϓ��ɉ����Ԃ�.
    a.position += n * s * 0.5f;
    b.position -= n * s * 0.5f;
  }
#endif
  return true;
}

/**
* �~���ƒ����̂̏Փ˂���������.
*
* @param a �Փˌ`�󂪉~���̃A�N�^�[.
* @param b �Փˌ`�󂪒����̂̃A�N�^�[.
* @param isBlock �ђʂ����Ȃ��ꍇtrue. �ђʂ���ꍇfalse
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
*/
bool CollideCylinderAndBox(Actor& a, Actor& b, bool isBlock)
{
  // �~���̉��[�������̂̏�[�̏�ɂ���Ȃ�Փ˂��Ă��Ȃ�.
  const float bottomA = a.position.y + a.collision.bottom;
  const float topB = b.position.y + b.collision.boxMax.y;
  if (bottomA >= topB) {
    return false;
  }
  // �~���̏�[�������̂̉��[�̉��ɂ���Ȃ�Փ˂��Ă��Ȃ�.
  const float topA = a.position.y + a.collision.top;
  const float bottomB = b.position.y + b.collision.boxMin.y;
  if (topA <= bottomB) {
    return false;
  }

  // �~���̒��S���ƒ����̂̍ł��߂�X����̈ʒupx���v�Z.
  float px = a.position.x;
  if (px < b.position.x + b.collision.boxMin.x) {
    px = b.position.x + b.collision.boxMin.x;
  } else if (px > b.position.x + b.collision.boxMax.x) {
    px = b.position.x + b.collision.boxMax.x;
  }

  // �~���̒��S���ƒ����̂̍ł��߂�Z����̈ʒupz���v�Z.
  float pz = a.position.z;
  if (pz < b.position.z + b.collision.boxMin.z) {
    pz = b.position.z + b.collision.boxMin.z;
  } else if (pz > b.position.z + b.collision.boxMax.z) {
    pz = b.position.z + b.collision.boxMax.z;
  }
  // �~���̒��S������ł��߂��_�܂ł̋�����2��(d2)���v�Z.
  const float dx = a.position.x - px;
  const float dz = a.position.z - pz;
  const float d2 = dx * dx + dz * dz;
  // d2���~���̔��a��2��ȏ�Ȃ�Փ˂��Ă��Ȃ�.
  if (d2 >= a.collision.radius * a.collision.radius) {
    return false;
  }

  // �u���b�N�w�肪�Ȃ���ΏՓ˂����Ƃ�����񂾂���Ԃ�.
  if (!isBlock) {
    return true;
  }

  // �u���b�N�w�肪����̂ŉ~���������Ԃ�.

#if 1
  // �����̂̒��S���W(cx, cz)���v�Z.
  const float cx = b.position.x +
    (b.collision.boxMin.x + b.collision.boxMax.x) * 0.5f;
  const float cz = b.position.z +
    (b.collision.boxMin.z + b.collision.boxMax.z) * 0.5f;

  // �����̂�X�y��Z�������̒��S���W����̒���(hx, hz)���v�Z.
  const float hx = (b.collision.boxMax.x - b.collision.boxMin.x) * 0.5f;
  const float hz = (b.collision.boxMax.z - b.collision.boxMin.z) * 0.5f;

  // �����̂̒��S���W����~���̒��S���܂ł̋���(ox, oz)���v�Z.
  const float ox = a.position.x - cx;
  const float oz = a.position.z - cz;

  // �����Ԃ�����p���v�Z.
  // ���xv��0�łȂ���Έړ������Ɍ������ʂ܂ŉ����Ԃ�.
  // 0�̏ꍇ�͉����Ԃ��������Z��������I��.
  const float r = a.collision.radius;
  glm::vec3 p = glm::vec3(0);

  if (ox < 0) {
    p.x = -(r + hx) - ox;
  } else {
    p.x = (r + hx) - ox;
  }

  const float y0 = bottomB - topA;
  const float y1 = topB - bottomA;
  if (std::abs(y0) <= std::abs(y1)) {
    p.y = y0;
  } else {
    p.y = y1;
  }
 
  if (oz < 0) {
    p.z = -(r + hz) - oz;
  } else {
    p.z = (r + hz) - oz;
  }

  // �����o���������ł��Z�������։����o��.
  const glm::vec3 absP = glm::abs(p);
  if (absP.x < absP.y) {
    if (absP.x < absP.z) {
      a.position.x += p.x;
      // �����Ԃ������Ƌt�����Ɉړ����Ă����瑬�x��0�ɂ���.
      if (a.velocity.x * p.x < 0) {
        a.velocity.x = 0;
      }
    } else {
      a.position.z += p.z;
      // �����Ԃ������Ƌt�����Ɉړ����Ă����瑬�x��0�ɂ���.
      if (a.velocity.z * p.z < 0) {
        a.velocity.z = 0;
      }
    }
  } else {
    if (absP.y < absP.z) {
      a.position.y += p.y;
      // �����Ԃ������Ƌt�����Ɉړ����Ă����瑬�x��0�ɂ���.
      if (a.velocity.y * p.y < 0) {
        a.velocity.y = 0;
      }
      // ��ɉ����Ԃ��ꍇ�͖��C�𔭐������A�������Ă�����Y�v�f��0�ɂ���.
      if (p.y >= 0) {
        a.drag = (a.friction + b.friction) * 0.5f * GameData::Get().gravity.y;
      }
    } else {
      a.position.z += p.z;
      // �����Ԃ������Ƌt�����Ɉړ����Ă����瑬�x��0�ɂ���.
      if (a.velocity.z * p.z < 0) {
        a.velocity.z = 0;
      }
    }
  }
  if (std::abs(a.position.x) > 100 || std::abs(a.position.y) > 100 || std::abs(a.position.z) > 100) {
    std::cerr << "[�x��]" << __func__ << "�v�Z�G���[\n";
  }
#else
  // Y�������̏d�Ȃ��Ă��镔���̒������v�Z.
  const float overlapY = std::min(topA, topB) - std::max(bottomA, bottomB);

  // �~���ƒ����̂̂����A�Z���ق��̍������v�Z.
  const float shortY = std::min(topA - bottomA, topB - bottomB);

  // �d�Ȃ��Ă��钷�����~���̔��������Ȃ��܂��͉��ɉ����Ԃ�.
  // �����ȏ�Ȃ牡�ɉ����Ԃ�.
  if (overlapY < shortY * 0.5f) {
    // �~���̉��[�������̂̉��[��荂���ʒu�ɂ���Ȃ�~������Ɉړ�.
    // �����łȂ���Ή��Ɉړ�.
    if (bottomA > bottomB) {
      a.position.y += topB - bottomA;
      a.velocity.y = std::max(a.velocity.y, 0.0f);
      a.drag = (a.friction + b.friction) * 0.5f * GameData::Get().gravity.y;
    } else {
      a.position.y -= topA - bottomB; // �~��vs�~���ƈႤ�̂Œ���.
      a.velocity.y = std::min(a.velocity.y, 0.0f);
    }
  } else if (d2 > 0) {
    // ���S���ƍŋߐړ_�̋���d���v�Z.
    const float d = std::sqrt(d2);
    // �����Ԃ�����s���v�Z.
    const float s = a.collision.radius - d;
    // �ŋߐړ_���璆�S���ւ̕����x�N�g��n���v�Z.
    const glm::vec3 n(dx / d, 0, dz / d);
    // �A�N�^�[A�������Ԃ�.
    a.position += n * s;
  } else {
    // �����̂̒��S���W(cx, cz)���v�Z.
    const float cx = b.position.x +
      (b.collision.boxMin.x + b.collision.boxMax.x) * 0.5f;
    const float cz = b.position.z +
      (b.collision.boxMin.z + b.collision.boxMax.z) * 0.5f;

    // �����̂�X�y��Z�������̒����̔���(hx, hz)���v�Z.
    const float hx = (b.collision.boxMax.x - b.collision.boxMin.x) * 0.5f;
    const float hz = (b.collision.boxMax.z - b.collision.boxMin.z) * 0.5f;

    // �����̂̒��S���W����~���̒��S���ւ̋���(ox, oz)���v�Z.
    const float ox = a.position.x - cx;
    const float oz = a.position.z - cz;

    // �����o���������v�Z.
    const float px = a.collision.radius + hx - std::abs(ox);
    const float pz = a.collision.radius + hz - std::abs(oz);

    // �����o���������Z���ق��ɉ����o��.
    if (px < pz) {
      // �~���̒��S����-X���ɂ���Ȃ�-X�����̈ړ������̂ق����Z��.
      if (ox < 0) {
        a.position.x -= px;
      } else {
        a.position.x += px;
      }
    } else {
      // �~���̒��S����-Z���ɂ���Ȃ�-Z�����̈ړ������̂ق����Z��.
      if (oz < 0) {
        a.position.z -= pz;
      } else {
        a.position.z += pz;
      }
    }
  }
#endif

  return true;
}

/**
* �����̓��m�̏Փ˂���������.
*
* @param a �Փˌ`�󂪒����̂̃A�N�^�[.
* @param b �Փˌ`�󂪒����̂̃A�N�^�[.
* @param isBlock �ђʂ����Ȃ��ꍇtrue. �ђʂ���ꍇfalse
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
*/
bool CollideBoxAndBox(Actor& a, Actor& b, bool isBlock)
{
  // ������a�̉��[��������b�̏�[�̏�ɂ���Ȃ�Փ˂��Ă��Ȃ�.
  const float bottomA = a.position.y + a.collision.boxMin.y;
  const float topB = b.position.y + b.collision.boxMax.y;
  if (bottomA >= topB) {
    return false;
  }
  // ������a�̏�[��������b�̉��[�̉��ɂ���Ȃ�Փ˂��Ă��Ȃ�.
  const float topA = a.position.y + a.collision.boxMax.y;
  const float bottomB = b.position.y + b.collision.boxMin.y;
  if (topA <= bottomB) {
    return false;
  }

  // ������a�̍��[��������b�̉E�[�̉E�ɂ���Ȃ�Փ˂��Ă��Ȃ�.
  const float leftA = a.position.x + a.collision.boxMin.x;
  const float rightB = b.position.x + b.collision.boxMax.x;
  if (leftA >= rightB) {
    return false;
  }
  // ������a�̉E�[��������b�̍��[�̍��ɂ���Ȃ�Փ˂��Ă��Ȃ�.
  const float rightA = a.position.x + a.collision.boxMax.x;
  const float leftB = b.position.x + b.collision.boxMin.x;
  if (rightA <= leftB) {
    return false;
  }

  // ������a�̎�O�[��������b�̉��[�̉��ɂ���Ȃ�Փ˂��Ă��Ȃ�.
  const float frontA = a.position.z + a.collision.boxMin.z;
  const float backB = b.position.z + b.collision.boxMax.z;
  if (frontA >= backB) {
    return false;
  }
  // ������a�̏�[��������b�̉��[�̉��ɂ���Ȃ�Փ˂��Ă��Ȃ�.
  const float backA = a.position.z + a.collision.boxMax.z;
  const float frontB = b.position.z + b.collision.boxMin.z;
  if (backA <= frontB) {
    return false;
  }

  return true;
}

/**
* �����ƕ��ʂ�����������W�����߂�.
*
*
*/
bool Intersect(const Segment& seg, const Plane& plane, glm::vec3* p)
{
  const float distance = glm::dot(plane.normal, plane.point - seg.start);
  const glm::vec3 v = seg.end - seg.start;

  // ���ꂪ�ق�0�̏ꍇ�A�����͕��ʂƕ��s�Ȃ̂Ō������Ȃ�.
  const float denom = glm::dot(plane.normal, v);
  if (std::abs(denom) < 0.0001f) {
    return false;
  }

  // ��_�܂ł̋���t��0�����܂���1���傫���ꍇ�A��_�͐����̊O���ɂ���̂Ŏ��ۂɂ͌������Ȃ�.
  const float t = distance / denom;
  if (t < 0 || t > 1) {
    return false;
  }

  // ��_�͐�����ɂ���.
  *p = seg.start + v * t;
  return true;
}

/**
* �������ʂ̕\���ɂ��邩�ǂ����𒲂ׂ�.
*
* @param sphere ��.
* @param plane  ����.
*
* @retval true  ���ʂ̕\���ɂ��邩�A�����I�ɏd�Ȃ��Ă���.
* @retval false ���S�ɗ����ɂ���.
*/
bool SphereInsidePlane(const Sphere& sphere, const Plane& plane)
{
  const float d = glm::dot(plane.normal, sphere.center - plane.point);
  return d >= -sphere.radius;
}

/**
* �~�������ʂ̕\���ɂ��邩�ǂ����𒲂ׂ�.
*
* @param cone   �~��.
* @param plane  ����.
*
* @retval true  ���ʂ̕\���ɂ��邩�A�����I�ɏd�Ȃ��Ă���.
* @retval false ���S�ɗ����ɂ���.
*/
bool ConeInsidePlane(const Cone& cone, const Plane& plane)
{
  if (glm::dot(plane.normal, cone.tip - plane.point) >= 0) {
    return true;
  }
  // ���ʂ̖@���Ɖ~���̌����ɐ����ȃx�N�g��a�����߂�.
  // ����͕��ʂɕ��s�ȃx�N�g���ɂȂ�.
  const glm::vec3 a = glm::cross(plane.normal, cone.direction);

  // �~���̌����ƃx�N�g��a�ɐ����ȃx�N�g�������߂�.
  // ����͒�ʂ̒��S���畽�ʂւ̍ŒZ�̌����x�N�g���ɂȂ�.
  const glm::vec3 b = glm::normalize(glm::cross(cone.direction, a));

  // �x�N�g��b�̕����̒�ʂ̒[�̍��W�����߂�.
  const glm::vec3 q = cone.tip +
    cone.direction * cone.height + b * cone.radius;

  // �x�N�g��c�̕��ʂ̖@�������Ɏˉe����������0�ȏ�Ȃ�q�͕\���ɂ���.
  return glm::dot(plane.normal, q - plane.point) >= 0;
}

/**
* �A�N�^�[�̏Փ˂���������.
*
* @param a �Փ˂���������A�N�^�[A.
* @param b �Փ˂���������A�N�^�[B.
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
*/
bool DetectCollision(Actor& a, Actor& b, bool block)
{
  // �A�N�^�[A�ƃA�N�^�[B�̗������ʂ蔲���֎~�Ȃ牟���Ԃ�
  const bool isBlock = block && a.collision.blockOtherActors && b.collision.blockOtherActors;

  // �Փˌ`�󂲂Ƃɏ����𕪂���.
  switch (a.collision.shape) {
  // �A�N�^�[A���~���̏ꍇ.
  case Collision::Shape::cylinder:
    switch (b.collision.shape) {
    case Collision::Shape::cylinder:
      return CollideCylinders(a, b, isBlock);
    case Collision::Shape::box:
      return CollideCylinderAndBox(a, b, isBlock);
    default:
      return false;
    }

  // �A�N�^�[A�������̂̏ꍇ.
  case Collision::Shape::box:
    switch (b.collision.shape) {
    case Collision::Shape::cylinder:
      return CollideCylinderAndBox(b, a, isBlock);
    case Collision::Shape::box:
      return CollideBoxAndBox(a, b, isBlock);
    default:
      return false;
    }

  // �A�N�^�[A������ȊO�̏ꍇ.
  default:
    return false;
  }
}

