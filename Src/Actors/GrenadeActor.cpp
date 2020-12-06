/**
* @file GrenadeActor.cpp
*/
#define NOMINMAX
#include "GrenadeActor.h"
#include "../MainGameScene.h"
#include "../GameData.h"
#include "../Audio.h"
#include "../Audio/MainWorkUnit/SE.h"

/**
* �R���X�g���N�^.
*
* @param pos  �A�N�^�[��z�u������W.
* @param vel  �A�N�^�[�̈ړ����x.
* @param rotY �A�N�^�[�̌���.
*/
GrenadeActor::GrenadeActor(const glm::vec3& pos, const glm::vec3& vel, float rotY, MainGameScene* pScene) :
  Actor("grenade",
    &GameData::Get().primitiveBuffer.Get(GameData::PrimNo::m67_grenade),
    std::make_shared<Texture::Image2D>("Res/m67_grenade.tga"),
    pos),
  pMainGameScene(pScene)
{
  rotation.y = rotY;
  velocity = vel;
  gravityScale = 1;
  lifespan = 2; // �_�Ύ���.
  //friction = 0.2f;

  // �Փˌ`���ݒ�.
  const float scaleFactor = 1;
  scale = glm::vec3(scaleFactor);
  SetCylinderCollision(0.1f * scaleFactor, -0.1f * scaleFactor, 0.1f * scaleFactor);
}

/**
* ��֒e�̏�Ԃ��X�V����.
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b).
*/
void GrenadeActor::OnUpdate(float deltaTime)
{
#if 0
  // �Փˉ񐔂ɉ����Č���.
  if (hitCount > 0) {
    // �Փ˂����񐔂����u���[�L��������.
    const float brakeSpeed = static_cast<float>(hitCount) * 4 * deltaTime;
    // �u���[�L���x��葬����Ό����A�u���[�L���x�ȉ��Ȃ瑬�x��0�ɂ���.
    if (glm::dot(velocity, velocity) > brakeSpeed * brakeSpeed) {
      // �t�x�N�g�����v�Z.
      const glm::vec3 vecInverse = -glm::normalize(velocity);
      // �u���[�L���x�Ɍo�ߎ��Ԃ��|�����l������.
      velocity += vecInverse * brakeSpeed;
    } else {
      velocity = glm::vec3(0);
    }
    // �Փˉ񐔂�0�ɖ߂�.
    hitCount = 0;
  }
#endif

  // ���x�ɉ����ĉ�]������.
  if (glm::dot(velocity, velocity) > 0) {
    const float speed = glm::length(velocity);
    rotation.z -= glm::radians(400.0f) * speed * deltaTime;
  }
}

/**
* ��֒e���j�󂳂ꂽ�Ƃ��̏���.
*/
void GrenadeActor::OnDestroy()
{
  // �U���͈̓A�N�^�[��ǉ�.
  ActorPtr actor = std::make_shared<Actor>("explosion", nullptr, nullptr, position);
  actor->SetCylinderCollision(3, -1, 3);
  actor->collision.blockOtherActors = false;
  actor->lifespan = 0.00001f;
  actor->OnHit = [](Actor& a, Actor& b) {
    if (b.name == "zombie" || b.name == "grenade") {
      // ������΂����A�N�^�[�̒��S���W���v�Z.
      glm::vec3 p = b.position;
      if (b.name == "zombie") {
        p.y += 1;
      }
      // ������ԕ������v�Z.
      const glm::vec3 v = p - a.position;
      // ���S�ɏd�Ȃ��Ă���ꍇ�͏�ɐ�����΂�.
      if (glm::dot(v, v) <= 0) {
        b.velocity += glm::vec3(0, 5, 0);
        return;
      }

      // �����̍ő哞�B�������v�Z.
      const float longY = std::max(a.collision.top, -a.collision.bottom);
      const float maxRange = std::sqrt(
        a.collision.radius * a.collision.radius + longY * longY);
      // �������S�ɋ߂��قǋ���������΂�.
      const float ratio = 1 - glm::length(v) / maxRange;
      // ������΂��͂͂Ƃ肠�����ő�6m/s�A�Œ�2m/s�Ƃ���.
      const float speed = ratio * 4 + 2;
      b.velocity += glm::normalize(v) * speed;
    }
  };
  pMainGameScene->AddActor(actor);

  Audio::Instance().Play(2, CRI_SE_BANG_2);
}
