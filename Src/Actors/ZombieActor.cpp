/**
* @file ZombieActor.cpp
*/
#include "ZombieActor.h"
#include "../GameData.h"
#include "../MainGameScene.h"

/**
* �R���X�g���N�^.
*
* @param pos    �A�N�^�[��z�u������W.
* @param rotY   �A�N�^�[�̌���.
* @param pScene ���C���Q�[���V�[���̃A�h���X.
*/
ZombieActor::ZombieActor(const glm::vec3& pos, float rotY,
  MainGameScene* pScene) :
  Actor("zombie", nullptr,
    std::make_shared<Texture::Image2D>("Res/zombie_male.tga"),
    pos),
  pMainGameScene(pScene)
{
  GameData& gamedata = GameData::Get();

  health = 5;

  gravityScale = 1;

  rotation.y = rotY;

  // �A�j���[�V������ݒ�.
  SetAnimation(gamedata.anmZombieMaleWalk);
  state = State::run;

  // �Փ˔����ݒ�.
  SetCylinderCollision(1.7f, 0, 0.5f);

  // �Փˏ�����ݒ�.
  OnHit = [](Actor& a, Actor& b) {
    if (b.name == "bullet") {
      // �ϋv�l�����炷.
      a.health -= 2;
      // �ϋv�l��0���傫����΃_���[�W�A�j���[�V�������Đ�����.
      // �ϋv�l��0�ȉ��ɂȂ����玀�S.
      if (a.health > 0) {
        // �m�b�N�o�b�N��ݒ肷��.
        if (glm::dot(b.velocity, b.velocity)) {
          a.velocity += glm::normalize(b.velocity) * 2.0f;
        }
        // �����A�j���͍Đ��ł��Ȃ��̂�nullptr���w�肵�ăA�j�����폜����.
        a.SetAnimation(nullptr);
        // �_���[�W�A�j���[�V�������Đ�.
        a.SetAnimation(GameData::Get().anmZombieMaleDamage);
        // �_���[�W��Ԃɐݒ�.
        a.state = Actor::State::damage;
      } else {
        // ���S�A�j���[�V������ݒ�.
        a.SetAnimation(GameData::Get().anmZombieMaleDown);
        // �Փ˔�����ɔ�������.
        a.SetCylinderCollision(0.01f, 0, 0.01f);
        // ���S��Ԃɐݒ�.
        a.state = Actor::State::dead;
        // �|�����]���r�̐���1�̑��₷.
        ++GameData::Get().killCount;
      }
    }
  };
}

/**
* �]���r�̏�Ԃ��X�V����.
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b).
*/
void ZombieActor::OnUpdate(float deltaTime)
{
  // �]���r�̍s��.
  // 1. +X�����ɒ��i.
  // 2. ���݌����Ă�������ɒ��i.
  // 3. �v���C���[�̕���������.
  // 4. �������v���C���[�̕���������.

  // �U�����ȊO�Ȃ�U���͈͂��폜����.
  if (attackActor && state != Actor::State::attack) {
    attackActor->isDead = true;
    attackActor = nullptr;
  }

  // �_���[�W��Ԃ̏ꍇ.
  if (state == Actor::State::damage) {
    // �A�j�����I��������ړ���Ԃɂ���.
    if (animationNo >= animation->list.size() - 1) {
      velocity = glm::vec3(0);
      SetAnimation(GameData::Get().anmZombieMaleWalk);
      state = Actor::State::run;
    }
  }
  // �U�����Ȃ�U���I����҂�.
  else if (state == Actor::State::attack) {
    // �A�j���[�V�����ԍ����A�j�������ȏゾ������A�U���A�j���I���Ƃ݂Ȃ�.
    if (animationNo >= animation->list.size() - 1) {
      SetAnimation(GameData::Get().anmZombieMaleWalk);
      state = Actor::State::run;
    }
    // �A�j���ԍ���4�ȏォ�U���͈͂����݂���΍U���͈͂��폜����.
    else if (animationNo >= 4) {
      if (attackActor) {
        attackActor->isDead = true;
        attackActor.reset();
      }
    }
    // �A�j���ԍ���3�ȏォ�U���͈͂����݂��Ȃ���΍U���͈͂��쐬����.
    else if (animationNo >= 3 && !attackActor) {
      // �]���r�̐��ʕ������v�Z.
      const glm::vec3 front(std::cos(rotation.y), 0, -std::sin(rotation.y));
      // �U������̔����ʒu���v�Z.
      const glm::vec3 pos = position + glm::vec3(0, 0.9f, 0) + front;
      // �U������A�N�^�[���쐬.
      attackActor = std::make_shared<Actor>("zombie_attack", nullptr, nullptr, pos);
      // �U�������ݒ�.
      attackActor->SetCylinderCollision(0.2f, -0.2f, 0.1f);
      attackActor->collision.blockOtherActors = false;
      pMainGameScene->AddActor(attackActor);
    }
  }

  // ����ł��Ȃ���Ε���.
  else if (state == Actor::State::run) {
    ActorPtr playerActor = pMainGameScene->GetPlayerActor();
    // �v���C���[�̂���������v�Z.
    glm::vec3 toPlayer = playerActor->position - position;
    // �]���r�̐��ʕ������v�Z.
    glm::vec3 front(std::cos(rotation.y), 0, -std::sin(rotation.y));
    // ���E�ǂ���ɉ�]���邩�����߂邽�߂ɊO�ς��v�Z.
    const glm::vec3 c = glm::cross(front, toPlayer);
    // �����x�N�g����y���W���v���X���Ȃ�����𑝂₵�A�}�C�i�X���Ȃ猸�炷.
    constexpr float speed = glm::radians(60.0f);
    if (c.y >= 0) {
      rotation.y += speed * deltaTime;
    } else {
      rotation.y -= speed * deltaTime;
    }
    // 360�x�𒴂�����0�x�ɖ߂�.
    constexpr float r360 = glm::radians(360.0f);
    rotation.y = fmod(rotation.y + r360, r360);
    // �������ω������̂ŁA���ʕ����̃x�N�g�����v�Z���Ȃ���.
    front.x = std::cos(rotation.y);
    front.z = -std::sin(rotation.y);
    // ���ʕ�����1m/s�̑��x�ňړ�����悤�ɐݒ�.
    velocity.x = front.x;
    velocity.z = front.z;

    // �v���C���[��������������3m�ȓ�������60�x�ȓ��ɂ�����U��.
    if (playerActor->state != Actor::State::dead) {
      const float distanceSq = glm::dot(toPlayer, toPlayer);
      if (distanceSq <= 3 * 3) {
        const float distance = std::sqrt(distanceSq);
        const float angle = std::acos(glm::dot(front, toPlayer * (1.0f / distance)));
        if (angle <= glm::radians(30.0f)) {
          SetAnimation(GameData::Get().anmZombieMaleAttack);
          state = Actor::State::attack;
        }
      }
    }
  } else {
    velocity.x = velocity.z = 0;
  }
}
