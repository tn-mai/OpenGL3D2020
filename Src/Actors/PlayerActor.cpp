/**
* @file PlayerActor.cpp
*/
#define NOMINMAX
#include "PlayerActor.h"
#include "GrenadeActor.h"
#include "../MainGameScene.h"
#include "../GameData.h"
#include "../Audio.h"
#include "../Audio/MainWorkUnit/SE.h"

/**
* �R���X�g���N�^.
*
* @param pos    �A�N�^�[��z�u������W.
* @param rotY   �A�N�^�[�̌���.
* @param pScene ���C���Q�[���V�[���̃A�h���X.
*/
PlayerActor::PlayerActor(const glm::vec3& pos, float rotY,
  MainGameScene* pScene) :
  Actor("player", nullptr,
    std::make_shared<Texture::Image2D>("Res/player_male.tga"),
    pos),
  pMainGameScene(pScene)
{
  // �d�͂̉e������ݒ�.
  gravityScale = 1;

  health = 10;
  SetAnimation(GameData::Get().anmPlayerIdle);
  SetCylinderCollision(1.7f, 0, 0.5f);
  OnHit = [](Actor& a, Actor& b) {
    if (b.name == "zombie_attack") {
      // ����ł����牽�����Ȃ�.
      if (a.state == Actor::State::dead) {
        return;
      }
      // ���G�^�C�}�[�ғ����͏Փ˂��Ȃ�.
      if (a.timer > 0) {
        return;
      }
      // �ϋv�͂����炷.
      a.health -= 1;
      b.collision.shape = Collision::Shape::none;
      // �ϋv�͂�0���傫����΃_���[�W�A�j���[�V�������Đ����A���G�^�C�}�[��ݒ�.
      // 0�ȉ��Ȃ玀�S.
      if (a.health > 0) {
        a.SetAnimation(GameData::Get().anmPlayerDamage);
        a.state = Actor::State::damage;
        a.timer = 2;
      } else {
        a.velocity = glm::vec3(0);
        a.timer = 3;
        a.SetAnimation(GameData::Get().anmPlayerDown);
        a.state = Actor::State::dead;
      }
    }
  };

  texBullet = std::make_shared<Texture::Image2D>("Res/Bullet.tga");
  texGrenade = std::make_shared<Texture::Image2D>("Res/m67_grenade.tga");
  texWoodenBarrior = std::make_shared<Texture::Image2D>("Res/wooden_barrier.tga");
}

/**
* �v���C���[�A�N�^�[�̓��͏���.
*/
void PlayerActor::ProcessInput()
{
  GameData& global = GameData::Get();

  // �v���C���[�A�N�^�[���ړ�������.
  glm::vec3 direction = glm::vec3(0);
  if (global.keyPressed & GameData::Key::left) {
    direction.x -= 1;
  } else if (global.keyPressed & GameData::Key::right) {
    direction.x += 1;
  }
  if (global.keyPressed & GameData::Key::up) {
    direction.z -= 1;
  } else if (global.keyPressed & GameData::Key::down) {
    direction.z += 1;
  }

  std::shared_ptr<Animation> nextAnime;
  if (glm::length(direction) > 0) {
    //playerActor->rotation.y = std::atan2(-direction.z, direction.x);
    const float speed = 4.0f;
    direction = glm::normalize(direction);
    velocity.x = direction.x * speed;
    velocity.z = direction.z * speed;

    const glm::vec3 front(std::cos(rotation.y), 0, -std::sin(rotation.y));
    const float cf = glm::dot(front, direction);
    if (cf > std::cos(glm::radians(45.0f))) {
      nextAnime = GameData::Get().anmPlayerRunFront;
    } else if (cf < std::cos(glm::radians(135.0f))) {
      nextAnime = GameData::Get().anmPlayerRunBack;
    } else {
      const glm::vec3 right(std::cos(rotation.y-glm::radians(90.0f)), 0, -std::sin(rotation.y-glm::radians(90.0f)));
      const float cr = glm::dot(right, direction);
      if (cr > std::cos(glm::radians(90.0f))) {
        nextAnime = GameData::Get().anmPlayerRunRight;
      } else {
        nextAnime = GameData::Get().anmPlayerRunLeft;
      }
    }
  } else {
    velocity.x = velocity.z = 0;
    nextAnime = GameData::Get().anmPlayerIdle;
  }

  // �W�����v.
  // maxH = v^2 / 2g
  // v = sqrt(maxH * 2g)
  if (GameData::Get().keyPressedInLastFrame & GameData::Key::jump) {
    velocity.y = 4.0f;
  }

  // �_���[�W�A�j���Đ����̓_���[�W�A�j�����I���܂ő҂�.
  if (animation == global.anmPlayerDamage) {
    if (animationNo >= animation->list.size() - 1) {
      SetAnimation(nextAnime);
    }
  } else {
    SetAnimation(nextAnime);
  }

  // ���˃L�[��������Ă�����O�_�˂��N��.
  if (GameData::Get().keyPressedInLastFrame & GameData::Key::shot) {
    leftOfRounds = maxRounds;
    shotTimer = 0;
  }
  // ���ː����c���Ă��Ĕ��˃^�C�}�[��0�ȉ��Ȃ�1������.
  if (leftOfRounds > 0 && shotTimer <= 0) {
    --leftOfRounds;
    shotTimer = shotInterval;

    // �v���C���[��Y����]���琳�ʕ������v�Z.
    const float fx = std::cos(rotation.y);
    const float fz = -std::sin(rotation.y); // Z���̌����͐��w�Ƌt.
    const glm::vec3 front = glm::vec3(fx, 0, fz);

    // �v���C���[��Y����]����E�������v�Z.
    const float rx = std::cos(rotation.y - glm::radians(90.0f));
    const float rz = -std::sin(rotation.y - glm::radians(90.0f)); // ����
    const glm::vec3 right = glm::vec3(rx, 0, rz);

    // �e�ۂ̔��ˈʒu(�e��)���v�Z. 3D���f���𒲂ׂ��Ƃ���A�e����
    // �v���C���[�̍��W(����)����O��0.6m�A�E��0.2m�A���0.9m�̈ʒu�ɂ���.
    const glm::vec3 bulletPosition =
      position + front * 0.6f + right * 0.2f + glm::vec3(0, 0.9f, 0);

    // �e�ۃA�N�^�[���e���̈ʒu�ɍ쐬.
    std::shared_ptr<Actor> bullet = std::make_shared<Actor>("bullet",
      &global.primitiveBuffer.Get(GameData::PrimNo::bullet), texBullet, bulletPosition);

    // ����(��])�̓v���C���[�A�N�^�[���p��.
    bullet->rotation = rotation;

    // front�����ցu���b20m�v�̑��x�ňړ�����悤�ɐݒ�.
    bullet->velocity = front * 20.0f;

    // �Փˌ`���ݒ�.
    bullet->SetCylinderCollision(0.1f, -0.1f, 0.125f);
    bullet->collision.blockOtherActors = false;

    // �Փˏ�����ݒ�.
    bullet->OnHit = [](Actor& a, Actor& b) {
      // �Փː悪�e�ۂ܂��̓v���C���[�̏ꍇ�͉������Ȃ�.
      if (b.name == "bullet" || b.name == "player") {
        return;
      }
      // �e�ۂ�����.
      a.isDead = true;
    };

    // �A�N�^�[�����X�g�ɒǉ�.
    pMainGameScene->AddActor(bullet);

    Audio::Instance().Play(1, CRI_SE_BANG_1);
  }

  // ��֒e�𓊂���.
  if (GameData::Get().keyPressedInLastFrame & GameData::Key::grenade) {
    // front�����ցu���b20m�v�̑��x�ňړ�����悤�ɐݒ�.
    // �v���C���[��Y����]���琳�ʕ������v�Z.
    const float fx = std::cos(rotation.y);
    const float fz = -std::sin(rotation.y); // Z���̌����͐��w�Ƌt.
    const glm::vec3 front = glm::vec3(fx, 0, fz);
    const glm::vec3 vel(front.x * 4, 4, front.z * 4);

    ActorPtr grenade = std::make_shared<GrenadeActor>(
      position + front * 0.6f + glm::vec3(0, 1.5f, 0), vel, rotation.y, pMainGameScene);
    pMainGameScene->AddActor(grenade);

    Audio::Instance().Play(1, CRI_SE_SLAP_0);
  }

  // �E�N���b�N�Ńo���P�[�h��z�u.
  if (!builderActor) {
    if (GameData::Get().keyPressed & GameData::Key::build) {
      builderActor = std::make_shared<Actor>("WoodenBarrior",
        &global.primitiveBuffer.Get(GameData::PrimNo::wooden_barrior), texWoodenBarrior, pMainGameScene->GetMouseCursor());
      // �Փˌ`���ݒ�.
      builderActor->SetBoxCollision(glm::vec3(-1, 0, -0.1f), glm::vec3(1, 2, 0.1f));
      builderActor->collision.blockOtherActors = false;
      builderActor->isShadowCaster = false;
      builderActor->health = 10;
      builderActor->OnHit = [](Actor& a, Actor& b) {
        a.baseColor = glm::vec4(1, 0.2f, 0.2f, 0.5f);
      };
      pMainGameScene->AddActor(builderActor);
    }
  }

  if (builderActor) {
    builderActor->position = pMainGameScene->GetMouseCursor();
    builderActor->position.y = 0;

    // �z�u������90���P�ʂŉ�].
    if (GameData::Get().keyPressed & GameData::Key::scrollup) {
      builderActor->rotation.y -= glm::radians(90.0f);
    }
    if (GameData::Get().keyPressed & GameData::Key::scrolldown) {
      builderActor->rotation.y += glm::radians(90.0f);
    }

    // �p�x�ɍ��킹�ďՓ˔����ݒ�.
    builderActor->rotation.y = std::fmod(builderActor->rotation.y + glm::radians(360.0f), glm::radians(360.0f));
    if (std::abs(builderActor->rotation.y - glm::radians(90.0f)) < glm::radians(5.0f)) {
      builderActor->SetBoxCollision(glm::vec3(-0.25f, 0, -1), glm::vec3(0.25f, 2, 1));
    } else if (std::abs(builderActor->rotation.y - glm::radians(270.0f)) < glm::radians(5.0f)) {
      builderActor->SetBoxCollision(glm::vec3(-0.25f, 0, -1), glm::vec3(0.25f, 2, 1));
    } else {
      builderActor->SetBoxCollision(glm::vec3(-1, 0, -0.25f), glm::vec3(1, 2, 0.25f));
    }

    // �L�[�������ꂽ��o���P�[�h��z�u.
    if (!(GameData::Get().keyPressed & GameData::Key::build)) {
      if (builderActor->baseColor.r < 1) {
        builderActor->baseColor = glm::vec4(1);
        builderActor->collision.blockOtherActors = true;
        builderActor->isShadowCaster = true;
        builderActor->OnHit = [](Actor& a, Actor& b) {
          if (b.name == "zombie_attack") {
            a.health -= 1;
            if (a.health <= 0) {
              a.isDead = true;
            }
            b.collision.shape = Collision::Shape::none;
          }
        };
      } else {
        builderActor->isDead = true;
      }
      builderActor.reset();
    }

    if (builderActor) {
      builderActor->baseColor = glm::vec4(0.2f, 0.2f, 1, 0.5f);
    }
  }

}

/**
* �v���C���[�A�N�^�[�̏�Ԃ��X�V����.
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b).
*/
void PlayerActor::OnUpdate(float deltaTime)
{
  if (leftOfRounds > 0) {
    shotTimer -= deltaTime;
  }
}

