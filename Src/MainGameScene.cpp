/**
* @file MainGameScene.cpp
*
* ����21���I�B�u�i�`�X2.0�v�̉Ȋw�����u�Q�����g�E�x���R�t�v(47)�́A�l�Ԃ��]���r�������p�u�f�X�E�X�g���N�`�������O�v�����p���B
* �i�`�X�����Ɛ��E�����̖�]�̎�n�߂ɁA�Ƃ��郈�[���b�p�̕�翂ȕГc�ɂ��]���r�܂݂�ɂ��悤�Ƃ��Ă����B
*
* SAS�\�����ŃT�o�C�o���E�C���X�g���N�^�[�́u�W�����E�X�~�X�v(31)�́A���܂��܉ċG�x�ɂŖK�ꂽ���ł��̊�݂Ɋ������܂�Ă��܂��B
* ���낤���ē������񂾒��̋���ŁA�q�t�́u�A���\�j�[�E�E�F�X�g�v��������ׂ�������������ꂽ�̂������B
* �u���Ȃ��͐_�����킳�ꂽ��m�Ȃ̂ł��B���́u�j�����ꂽAK-47�v�Ɓu���Ȃ��֒e�v�������Ȃ����B�v
* �u�����Đ_�̉h���������āA�ׂȎ��҂ǂ��Ɉ����������炷�̂ł��B�v
* �W�����͋����Ȃ�����u��烂̎s�������̂��R�l�̖��߁B�v�Ƃ����㊯�u�G�h�K�[�E���b�Z���v(35)�̋������v���o���A�^���ɏ]�����Ƃɂ���B
* �i�`�X2.0�̖�]�����������߁A�r��ʂĂ����ƃ]���r�̑��H���[���X���삯������W�����̉ߍ��Ȑ킢���n�܂����I
*
* �u�����͂���˂��B�z��͂Ƃ����Ɏ���ł�񂾁B�v
*
* [����̓��e���]
*
* ������:
* - �z���[�����u�������d��.
* - �z���[���̓]���r�̑�Q�Ƃ������_�ŏ\��.
* - �h��Ȕ����A����A�U���A�����Ĕ�юU�錌���Ԃ�.
*
* �v���O�����̋����_:
* - �T�E���h.
* - �}�E�X�ɂ��Ə��Ǝˌ�. �������ƏՓː}�`�Ƃ̏Փ˔��肪�K�v.
* - ���̏Փ˔���.
* - Y����]���������̂̏Փ˔���.
* - �t�F�[�h�C���E�A�E�g.
* - ���[�t�B���O�ɂ��⊮�A�j���[�V����
* - �I�t�X�N���[���T�[�t�F�X�ɂ���ʌ���.
* - �m�[�}���}�b�v.
* - �O�p�|���S���E�l�p�|���S���ƒ����̏Փ˔���.
* - ���f���f�[�^�̌y�ʉ�.
* - ��p�̃��f���t�H�[�}�b�g�̊J��.
*
* �v���C���[�̋@�\:
* - ��֒e�𓊂���. �]����(�ǉ��̃A�j���[�V�������K�v).
* - �������ւ̍U����i.
* - �o���P�[�h�쐬.
* - �O�]�A�U��قǂ��A�W�����v�A������у_�E���A�_�E������̋N���オ��.
* - �ߐڍU��.
*
* �]���r�̋@�\:
* - �]���r���f���̃o���G�[�V�����𑝂₷.
* - �����g����or�����ɂ�陳���ړ�����эU��.
* - ������у_�E��.
* - �_�E������̋N���オ��.
* - �������U����̃_�E��.
* - �㔼�g���U����̃_�E��.
* - ����.
* - ����_�E��.
* - ���݂��U��.
* - ��������܂��̃|�[�Y.
* - �{�X�]���r.
*/
#define NOMINMAX
#include "MainGameScene.h"
#include "GameData.h"
#include "SceneManager.h"
#include "Actors/PlayerActor.h"
#include "Actors/ZombieActor.h"
#include "Audio.h"
#include "Audio/MainWorkUnit/SE.h"
#include "Audio/MainWorkUnit/BGM.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

/**
* ���؂�`�悷��.
*/
void MainGameScene::AddLineOfTrees(const glm::vec3& start, const glm::vec3& direction)
{
  GameData& global = GameData::Get();

  glm::vec3 offset = start;
  for (float i = 0; i < 19; ++i) {
    std::shared_ptr<Actor> actor = std::make_shared<Actor>("tree",
      &global.primitiveBuffer.Get(GameData::PrimNo::tree),
      texTree, start + direction * i);
    actor->rotation.y = glm::radians(i * 30);
    actor->SetBoxCollision(glm::vec3(-1, 0, -1), glm::vec3(1, 100, 1));
    actors.push_back(actor);
  }
}

/**
* ���C���Q�[����ʂ�����������.
*
* @retval true  ����������.
* @retval false ���������s.
*/
bool MainGameScene::Initialize()
{
  // FBO������������.
  int w, h;
  glfwGetFramebufferSize(GameData::Get().window, &w, &h);
  fbo = std::make_shared<FramebufferObject>(w, h, FboType::ColorDepthStencil);
  if (!fbo || !fbo->GetId()) {
    return false;
  }

  // �f�v�V���h�E�}�b�s���O�p��FBO���쐬.
  fboShadow = std::make_shared<FramebufferObject>(4096, 4096, FboType::Depth);
  if (!fboShadow || !fboShadow->GetId()) {
    return false;
  }

  // �u���[���G�t�F�N�g�p��FBO���쐬.
  int bloomW = w;
  int bloomH = h;
  for (size_t i = 0; i < std::size(fboBloom); ++i) {
    bloomW /= 2;
    bloomH /= 2;
    fboBloom[i]= std::make_shared<FramebufferObject>(bloomW, bloomH, FboType::Color);
    if (!fboBloom[i]|| !fboBloom[i]->GetId()) {
      return false;
    }
  }

  texGround = std::make_shared<Texture::Image2D>("Res/Ground.tga");
  texTree   = std::make_shared<Texture::Image2D>("Res/Tree.tga");
  texHouse  = std::make_shared<Texture::Image2D>("Res/House.tga");
  texCube   = std::make_shared<Texture::Image2D>("Res/Rock.tga");
  texZombie = std::make_shared<Texture::Image2D>("Res/zombie_male.tga");
  texPlayer = std::make_shared<Texture::Image2D>("Res/player_male.tga");
  texGameClear = std::make_shared<Texture::Image2D>("Res/Survived.tga");
  texGameOver = std::make_shared<Texture::Image2D>("Res/GameOver.tga");
  texBlack = std::make_shared<Texture::Image2D>("Res/Black.tga");
  texPointer = std::make_shared<Texture::Image2D>("Res/Pointer.tga");
  texStreetLampA = std::make_shared<Texture::Image2D>("Res/street_lamp_a.tga");
  texStreetLampBC = std::make_shared<Texture::Image2D>("Res/street_lamp_bc.tga");
  texStreetLampD = std::make_shared<Texture::Image2D>("Res/street_lamp_d.tga");

  GameData& global = GameData::Get();

  // �n�ʂ�\��.
  {
    std::shared_ptr<Actor> actor = std::make_shared<Actor>(
      "ground", &global.primitiveBuffer.Get(GameData::PrimNo::ground), texGround, glm::vec3(0, 0, 0));
    actor->samplers[0] = std::make_shared<Texture::Sampler>();
    actor->samplers[0]->SetFilter(GL_LINEAR);
    actor->samplers[0]->SetWrapMode(GL_REPEAT);
    actor->samplers[1] = actor->samplers[0];
    actor->samplers[2] = actor->samplers[0];
    actor->texNormal = std::make_shared<Texture::Image2D>("Res/Ground_normal.tga", Texture::ImageType::non_color);
    actor->texMetallicSmoothness =
      std::make_shared<Texture::Image2D>("Res/Ground_spec.tga", Texture::ImageType::non_color);
    actor->SetBoxCollision(glm::vec3(-20, -10, -20), glm::vec3(20, 0, 20));
    actor->isShadowCaster = false;
    actors.push_back(actor);
  }

  // �؂�\��.
  for (float j = 0; j < 4; ++j) {
    const glm::mat4 matRot = glm::rotate(glm::mat4(1), glm::radians(90.0f) * j, glm::vec3(0, 1, 0));
    AddLineOfTrees(matRot * glm::vec4(-19, 0, 19, 1), matRot * glm::vec4(2, 0, 0, 1));
  }

  // �Ƃ�\��.
  {
    std::shared_ptr<Actor> actor = std::make_shared<Actor>(
      "house", &global.primitiveBuffer.Get(GameData::PrimNo::house), texHouse, glm::vec3(0));
    actor->samplers[0] = std::make_shared<Texture::Sampler>();
    actor->samplers[0]->SetFilter(GL_NEAREST);
    actor->samplers[0]->SetWrapMode(GL_CLAMP_TO_EDGE);
    actor->samplers[2] = actor->samplers[0];
    //actor->texNormal = std::make_shared<Texture::Image2D>("Res/house_normal.tga");
    //actor->texMetallicSmoothness = std::make_shared<Texture::Image2D>("Res/House_spec.tga", false);
    actor->SetBoxCollision(glm::vec3(-3, 0, -3), glm::vec3(3, 5, 3));
    actors.push_back(actor);
  }

  // �����̂�\��.
  {
    std::shared_ptr<Actor> actor = std::make_shared<Actor>(
      "cube", &global.primitiveBuffer.Get(GameData::PrimNo::cube), texCube, glm::vec3(10, 1, 0));
    actor->SetBoxCollision(glm::vec3(-1), glm::vec3(1));
    actors.push_back(actor);
  }

  // �v���C���[��\��.
  {
    playerActor = std::make_shared<PlayerActor>(glm::vec3(10, 0, 10), 0.0f, this);
    actors.push_back(playerActor);
  }

  // �]���r��\��.
  const Mesh::Primitive* pPrimitive = &global.primitiveBuffer.Get(GameData::PrimNo::zombie_male_walk_0);
  for (size_t i = 0; i < appearanceEnemyCount; ++i) {
    glm::vec3 pos(0);
    pos.x = std::uniform_real_distribution<float>(-18, 18)(global.random);
    pos.z = std::uniform_real_distribution<float>(-18, 18)(global.random);
    float rotY =
      std::uniform_real_distribution<float>(0, glm::radians(360.0f))(global.random);
    std::shared_ptr<Actor> actor = std::make_shared<ZombieActor>(pos, rotY, this);
    actors.push_back(actor);
  }

  // �_������ݒ肷��
  pointLight = Shader::PointLight{
    glm::vec4(8, 10,-8, 0),
    glm::vec4(0.4f, 0.7f, 1.0f, 0) * 200.0f
  };

  lightManager = std::make_shared<Light::LightManager>();

  // ���C�g�t���X�^�����쐬.
  {
    GameData& gamedata = GameData::Get();
    int w, h;
    glfwGetWindowSize(GameData::Get().window, &w, &h);
    const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    matProj = glm::perspective(glm::radians(45.0f), aspectRatio, 1.0f, 500.0f);
    frustum = Light::CreateFrustum(matProj, 1, 500);

    for (int i = 0; i < 100; ++i) {
      //const float x = std::uniform_real_distribution<float>(-18, 18)(gamedata.random);
      //const float y = std::uniform_real_distribution<float>(0.5f, 3)(gamedata.random);
      //const float z = std::uniform_real_distribution<float>(-18, 18)(gamedata.random);
      const float x = static_cast<float>(i % 10) * 4 - 18;
      const float z = static_cast<float>(i / 10) * 4 - 18;
      const int c = std::uniform_int_distribution<>(1, 7)(gamedata.random);
      const float range = std::uniform_real_distribution<float>(0.5f, 1.5f)(gamedata.random);

      const glm::vec3 color = glm::vec3(c & 1, (c >> 1) & 1, (c >> 2) & 1) * range * range * 3.14f;
      lightManager->CreateLight(glm::vec3(x, 1.0f, z), color);
    }
    flashLight = lightManager->CreateSpotLight(playerActor->position + glm::vec3(0, 1, 0),
      glm::vec3(1, 0.9f, 0.8f) * 30.0f, glm::vec3(1, 0, 0),
      glm::radians(20.0f), glm::radians(5.0f));
  }

  // �X����\��
  if (1) {
    static const glm::vec3 pos[] = {
      { -16, 0, -16 }, { -16, 0, -10 }, { -16, 0, -4 },
      { -16, 0,   2 }, { -16, 0,   8 }, { -16, 0, 14 },
      { -6,  0,   6 }, {  6, 0, 6},
    };
    for (const auto& e : pos) {
      std::shared_ptr<Actor> actor = std::make_shared<Actor>(
        "StreetLamp", &global.primitiveBuffer.Get(GameData::PrimNo::street_lamp_a), texStreetLampA, e);
      actor->SetBoxCollision(glm::vec3(-0.2f, 0, -0.2f), glm::vec3(0.2f, 4, 0.2f));
      //actor->rotation.y = glm::radians(180.0f);
      actors.push_back(actor);
      if (e.x == 6) {
        actor->rotation.y = 3.1415f;
        lightManager->CreateSpotLight(e + glm::vec3(-1, 4, 0), glm::vec3(1, 0.9f, 0.8f) * 100.0f, glm::vec3(0, -1, 0),
          glm::radians(30.0f), glm::radians(15.0f));
      } else {
        lightManager->CreateSpotLight(e + glm::vec3(1, 4, 0), glm::vec3(1, 0.9f, 0.8f) * 100.0f, glm::vec3(0, -1, 0),
          glm::radians(30.0f), glm::radians(15.0f));
      }
    }
  }

  // �Q�[���f�[�^�̏����ݒ�.
  GameData& gamedata = GameData::Get();
  gamedata.killCount = 0;

  // �}�E�X�J�[�\�����\���ɂ���.
  glfwSetInputMode(gamedata.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

  // �V�K�A�N�^�[�ǉ��p�̔z��̃T�C�Y��\�񂷂�.
  newActors.reserve(100);

  sprites.reserve(1000);
  spriteRenderer.Allocate(1000);

  Audio::Instance().Play(4, CRI_BGM_BENSOUND_HIGHOCTANE);

  std::cout << "[���] MainGameScene���J�n.\n";
  return true;
}

/**
* ���C���Q�[����ʂ̃L�[���͂���������.
*
* @param window    GLFW�E�B���h�E�ւ̃|�C���^.
*/
void MainGameScene::ProcessInput(GLFWwindow* window)
{
  // �N���A���Ă���?
  GameData& gamedata = GameData::Get();
  if (isGameClear) {
    // Enter�L�[�������ꂽ��^�C�g����ʂɈړ�.
    if (gamedata.keyPressedInLastFrame & GameData::Key::enter) {
      SceneManager::Get().ChangeScene(TITLE_SCENE_NAME);
    }
    return;
  }

  // �Q�[���I�[�o�[?
  if (isGameOver) {
    // Enter�L�[�������ꂽ��^�C�g����ʂɈړ�.
    if (gamedata.keyPressedInLastFrame & GameData::Key::enter) {
      SceneManager::Get().ChangeScene(TITLE_SCENE_NAME);
    }
    return;
  }

  // �v���C���[������ł�����
  if (!isGameOver) {
    if (playerActor->state == Actor::State::dead) {
      if (flashLight) {
        lightManager->RemoveLight(flashLight);
        flashLight.reset();
      }
      // �A�j���[�V�������I�����Ă�����Q�[���I�[�o�[�ɂ���.
      if (playerActor->animationNo >= playerActor->animation->list.size() - 1) {
        Audio::Instance().Stop(4);
        Audio::Instance().Play(4, CRI_BGM_GAME_OVER);
        isGameOver = true;
      }
      return;
    }
  }

  // �}�E�X�|�C���^��n�ʂɕ\��.
  {
    const glm::vec2 cursor = GameData::Get().cursorPosition;

    Segment seg;
    const glm::vec2 screenPosition((cursor.x / 1280) * 2, (cursor.y / 720) * 2);
    const glm::mat4 matInverseVP = glm::inverse(matProj * matView);

    const glm::vec4 start = matInverseVP * glm::vec4(screenPosition, -1, 1);
    seg.start = glm::vec3(start) / start.w;

    const glm::vec4 end = matInverseVP * glm::vec4(screenPosition, 1, 1);
    seg.end = glm::vec3(end) / end.w;

    const Plane plane{ playerActor->position + glm::vec3(0, 1, 0), glm::vec3(0, 1, 0) };
    Intersect(seg, plane, &posMouseCursor);

    const glm::vec3 direction(posMouseCursor - playerActor->position);
    playerActor->rotation.y = std::atan2(-direction.z, direction.x);
  }

  playerActor->ProcessInput();
  if (!newActors.empty()) {
    actors.insert(actors.end(), newActors.begin(), newActors.end());
    newActors.clear();
  }
}

/**
* ���C���Q�[����ʂ��X�V����.
*
* @param window    GLFW�E�B���h�E�ւ̃|�C���^.
* @param deltaTime �O��̍X�V����̌o�ߎ���.
*/
void MainGameScene::Update(GLFWwindow* window, float deltaTime)
{
  const float maxDeltaTime = 1.0f / 30.0f;
  remainingDeltaTime += deltaTime;
  deltaTime = std::min(remainingDeltaTime, maxDeltaTime);
  do {
    // �A�N�^�[���X�g�Ɋ܂܂��A�N�^�[�̏�Ԃ��X�V����.
    UpdateActorList(actors, deltaTime);

    // �V�K�A�N�^�[�����݂���Ȃ�A������A�N�^�[���X�g�ɒǉ�����.
    if (!newActors.empty()) {
      actors.insert(actors.end(), newActors.begin(), newActors.end());
      newActors.clear();
    }

    // �Փ˔���.

    // �Փ˂���\���̂���y�A���i�[����z��.
    std::vector<std::pair<Actor&, Actor&>> potentiallyCollidingPairs;
    potentiallyCollidingPairs.reserve(1000);

    // �Փ˂����y�A���i�[����z��.
    std::vector<std::pair<Actor*, Actor*>> collidedPairs;
    collidedPairs.reserve(1000);

    // �Փ˔�������A�N�^�[���������o��.
    ActorList actorsWithCollider;
    actorsWithCollider.reserve(actors.size());
    std::copy_if(
      actors.begin(), actors.end(), std::back_inserter(actorsWithCollider),
      [](const ActorPtr& a) { return a->collision.shape != Collision::Shape::none; });

    // �Փ˂������ȃy�A�����X�g�A�b�v.
    for (size_t ia = 0; ia < actorsWithCollider.size(); ++ia) {
      Actor& a = *actorsWithCollider[ia]; // �A�N�^�[A
      // �v�Z�ς݋y�ю������g�������A�c��̃A�N�^�[�Ƃ̊ԂŏՓ˔�������s.
      for (size_t ib = ia + 1; ib < actorsWithCollider.size(); ++ib) {
        Actor& b = *actorsWithCollider[ib]; // �A�N�^�[B

        // 2�̏Փ�AABB���������Ă��Ȃ���ΏՓ˂��Ȃ�(����).
        const glm::vec3 d = glm::abs(b.boundingBox.c - a.boundingBox.c);
        const glm::vec3 r = a.boundingBox.r + b.boundingBox.r;
        if (d.x > r.x || d.y > r.y || d.z > r.z) {
          continue;
        }

        // �ǂ��炩���u���b�N���Ȃ��A�܂��͏Փˌ`�󂪒����̓��m�̏ꍇ�͂����ŏՓ˔�����s��.
        if (!a.collision.blockOtherActors || !b.collision.blockOtherActors || (
          a.collision.shape == Collision::Shape::box &&
          b.collision.shape == Collision::Shape::box)) {
          if (DetectCollision(a, b, true)) {
            collidedPairs.emplace_back(&a, &b);
          }
          continue;
        }

        // �����܂ŗ�����Փ˂̉\��������.
        potentiallyCollidingPairs.emplace_back(a, b);
      }
    }

    // �{���ɏՓ˂��邩�𒲂ׂ�.
    for (size_t loop = 0; loop < 5; ++loop) {
      bool hasCollision = false;
      for (auto& pair : potentiallyCollidingPairs) {
          if (DetectCollision(pair.first, pair.second, true)) {
            collidedPairs.emplace_back(&pair.first, &pair.second);
            hasCollision = true;
          }
      }
      if (!hasCollision) {
        break;
      }
    }

    // �d������y�A�����O����.
    std::sort(collidedPairs.begin(), collidedPairs.end());
    const auto itr = std::unique(collidedPairs.begin(), collidedPairs.end());
    collidedPairs.erase(itr, collidedPairs.end());

    // OnHit���Ăяo��.
    for (auto& e : collidedPairs) {
      e.first->OnHit(*e.first, *e.second);
      e.second->OnHit(*e.second, *e.first);
    }

    // ����̍X�V�ŏ�������Ԃ��c��o�ߎ��Ԃ��猸�Z.
    // �o�ߎ��Ԃ��Œ�l�ȏ�Ȃ烋�[�v.
    remainingDeltaTime -= deltaTime;
  } while (remainingDeltaTime >= maxDeltaTime);

  // �܂��N���A���Ă��Ȃ�?
  if (!isGameClear) {
    // �N���A����(�u�|�����G�̐��v���u�o������G�̐��v�ȏ�)�𖞂����Ă���?
    if (GameData::Get().killCount >= appearanceEnemyCount) {
      // �Q�[���N���A�t���O��true�ɂ���.
      isGameClear = true;

      // �v���C���[�A�N�^�[��ҋ@��Ԃɂ���.
      playerActor->velocity = glm::vec3(0);
      playerActor->SetAnimation(GameData::Get().anmPlayerIdle);

      Audio::Instance().Stop(4);
      Audio::Instance().Play(4, CRI_BGM_SUCCESS);

      std::cout << "[���] �Q�[���N���A������B��\n";
    }
  }

  // �J�������v���C���[�A�N�^�[�̂ȂȂߏ�ɔz�u.
  const glm::vec3 viewPosition = playerActor->position + glm::vec3(0, 7, 7);
  // �v���C���[�A�N�^�[�̑�������ʂ̒����ɉf��悤�ɂ���.
  const glm::vec3 viewTarget = playerActor->position;

  // ���W�ϊ��s����쐬.
  int w, h;
  glfwGetWindowSize(window, &w, &h);
  const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
  matProj =
    glm::perspective(glm::radians(45.0f), aspectRatio, 1.0f, 500.0f);
  matView =
    glm::lookAt(viewPosition, viewTarget, glm::vec3(0, 1, 0));

  UpdateSpriteList(sprites, deltaTime);
  spriteRenderer.Update(sprites, matView);

  if (flashLight) {
    flashLight->position = playerActor->position + glm::vec3(0, 1.2f, 0);
    flashLight->direction = glm::rotate(glm::mat4(1),
      playerActor->rotation.y, glm::vec3(0, 1, 0)) * glm::vec4(1, -0.1f, 0, 1);
    flashLight->direction = glm::normalize(flashLight->direction);
  }
  lightManager->Update(matView, frustum);
}

/**
* ���C���Q�[����ʂ�`�悷��.
*
* @param window    GLFW�E�B���h�E�ւ̃|�C���^.
*/
void MainGameScene::Render(GLFWwindow* window) const
{
  int fbw, fbh;
  glfwGetFramebufferSize(window, &fbw, &fbh);
  if (fbw <= 0 || fbh <= 0) {
    return;
  }

  GameData& global = GameData::Get();
  GameData& gamedata = GameData::Get();
  std::shared_ptr<Shader::Pipeline> pipeline = global.pipeline3D;
  Mesh::PrimitiveBuffer& primitiveBuffer = global.primitiveBuffer;

#define TIME_SETTING_NOON 0
#define TIME_SETTING_SUNSET 1
#define TIME_SETTING_NIGHT 2

#define TIME_SETTING TIME_SETTING_NOON

  const Shader::DirectionalLight directionalLight{
    glm::normalize(glm::vec4(3,-2,-2, 0)),
#if TIME_SETTING == TIME_SETTING_NOON
    glm::vec4(glm::vec3(1, 0.9f, 0.8f) * 4.0f, 1) // ��
#elif TIME_SETTING == TIME_SETTING_SUNSET
    glm::vec4(glm::vec3(1, 0.5f, 0.2f) * 2.0f, 1) // �[��
#else // TIME_SETTING == TIME_SETTING_NIGHT
    glm::vec4(glm::vec3(1, 0.9f, 0.8f) * 0.25f, 1) // ��
#endif
  };

  // �e�`��pFBO�ɕ`�悷��.
  {
    // �`�����e�`��pFBO�ɕύX.
    fboShadow->Bind();
    glDisable(GL_FRAMEBUFFER_SRGB); // �K���}�␳�𖳌��ɂ���.
    glDisable(GL_BLEND); // �A���t�@�u�����h�𖳌��ɂ���.
    glEnable(GL_DEPTH_TEST); // �[�x�e�X�g��L���ɂ���.
    glEnable(GL_CULL_FACE); // ���ʃJ�����O��L���ɂ���.

    // �[�x�o�b�t�@���N���A.
    glClear(GL_DEPTH_BUFFER_BIT);

    primitiveBuffer.BindVertexArray();
    std::shared_ptr<Shader::Pipeline> pipelineShadow = gamedata.pipelineShadow;
    pipelineShadow->Bind();
    gamedata.sampler.Bind(0);

    // ���W�ϊ��s����쐬.
    const float halfW = 50;
    const float halfH = 50;
    glm::mat4 matProj =
      glm::ortho(-halfW, halfW, -halfH, halfH, 20.0f, 200.0f);
      //glm::perspective(glm::radians(30.0f), 1.0f, 20.0f, 200.0f);

    const glm::vec3 viewPosition = playerActor->position - glm::vec3(directionalLight.direction) * 100.0f;
    //const glm::vec3 viewPosition = -glm::vec3(directionalLight.direction) * 50.0f;
    const glm::vec3 viewTarget = playerActor->position;
    glm::mat4 matView =
      glm::lookAt(viewPosition, viewTarget, glm::vec3(0, 1, 0));

    glm::mat4 matShadowVP = matProj * matView;
    for (size_t i = 0; i < actors.size(); ++i) {
      actors[i]->Draw(*pipelineShadow, matShadowVP, Actor::DrawType::shadow);
    }

    // �N���b�v���W�n����X�N���[�����W�n�ւ̍��W�ϊ��s���ǉ�.
    const glm::mat4 matBiasVP = glm::mat4(
      0.5f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.5f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.5f, 0.0f,
      0.5f, 0.5f, 0.5f, 1.0f
    ) * matShadowVP;
    pipeline->SetShadowMatrix(matBiasVP);
    gamedata.pipelineDeathEffect->SetShadowMatrix(matBiasVP);
  }

  // �`�����t���[���o�b�t�@�I�u�W�F�N�g�ɕύX.
  fbo->Bind();
  glDisable(GL_FRAMEBUFFER_SRGB); // �K���}�␳�𖳌��ɂ���

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // ������ݒ肷��.
#if TIME_SETTING == TIME_SETTING_NOON
  pipeline->SetAmbientLight(glm::vec3(0.1f, 0.125f, 0.15f)); // ��
#elif TIME_SETTING == TIME_SETTING_SUNSET
  pipeline->SetAmbientLight(glm::vec3(0.09f, 0.05f, 0.1f)); // �[�� 
#else // TIME_SETTING == TIME_SETTING_NIGHT
  pipeline->SetAmbientLight(glm::vec3(0.02f, 0.01f, 0.03f)); // ��
#endif

  // ���s������ݒ肷��
  pipeline->SetLight(directionalLight);

  //pipeline->SetLight(pointLight);
  // ���C�g�pSSBO���o�C���h.
  lightManager->Bind(0);

  primitiveBuffer.BindVertexArray();
  pipeline->Bind();
  global.sampler.Bind(0);
  global.sampler.Bind(1);
  global.samplerClampToEdge.Bind(2);
  global.samplerClampToEdge.Bind(3);
  global.samplerShadow.Bind(4);

  // �A�N�^�[���X�g��`��.
  pipeline->SetViewPosition(playerActor->position + glm::vec3(0, 7, 7));

  std::shared_ptr<Shader::Pipeline> pipelineDeathEffect = global.pipelineDeathEffect;
  pipelineDeathEffect->SetAmbientLight(glm::vec3(0.1f, 0.125f, 0.15f));
  pipelineDeathEffect->SetLight(directionalLight);
  pipelineDeathEffect->SetViewPosition(playerActor->position + glm::vec3(0, 7, 7));

  // �V���h�E�e�N�X�`�����o�C���h.
  fboShadow->BindDepthStencilTexture(4);

  const glm::mat4 matVP = matProj * matView;
  for (size_t i = 0; i < actors.size(); ++i) {
    actors[i]->Draw(matVP, Actor::DrawType::color);
  }

  global.samplerShadow.Unbind();
  global.samplerClampToEdge.Bind(2);
  global.samplerClampToEdge.Bind(3);

  if (0) {
    // Y����].
    const float degree = static_cast<float>(std::fmod(glfwGetTime() * 180.0, 360.0));
    const glm::mat4 matModelR =
      glm::rotate(glm::mat4(1), glm::radians(degree), glm::vec3(0, 1, 0));
    // �g��k��.
    const glm::mat4 matModelS =
      glm::scale(glm::mat4(1), glm::vec3(0.5f, 0.25f, 0.5f));
    // ���s�ړ�.
    const glm::mat4 matModelT =
      glm::translate(glm::mat4(1), glm::vec3(pointLight.position) + glm::vec3(0, -1.25f, 0));
    // �g��k���E��]�E���s�ړ�������.
    const glm::mat4 matModel = matModelT * matModelR * matModelS;
    pipeline->SetMVP(matProj * matView * matModel);
    pipeline->SetModelMatrix(matModel);
    texTree->Bind(0);
    primitiveBuffer.Get(GameData::PrimNo::tree).Draw();
  }

  // �A�N�^�[�̉e��`��.
  if (0) {
    // �X�e���V���o�b�t�@��L���ɂ���.
    glEnable(GL_STENCIL_TEST);
    // �u��r�Ɏg���l�v��1�ɂ��āA��ɔ�r����������悤�ɐݒ�.
    glStencilFunc(GL_ALWAYS, 1, 0xff);
    // �X�e���V���[�x�̗����̃e�X�g�ɐ��������ꍇ�Ɂu��r����l�v���������ނ悤�ɐݒ�.
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    // �X�e���V���o�b�t�@�̑S�r�b�g�̏������݂�����.
    glStencilMask(0xff);
    // �J���[�o�b�t�@�ւ̏������݂��֎~.
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    // �[�x�o�b�t�@�ւ̏������݂��֎~.
    glDepthMask(GL_FALSE);

    // ����1m�̕��̂����Ƃ��e�̒������v�Z.
    const float scale = 1.0f / -directionalLight.direction.y;
    const float sx = directionalLight.direction.x * scale;
    const float sz = directionalLight.direction.z * scale;

    // �؂���񂱍s��(Y���W��0�ɂ���s��)���쐬.
    const glm::mat4 matShadow(
      1.00f, 0.00f, 0.00f, 0.00f,
      sx, 0.00f, sz, 0.00f,
      0.00f, 0.00f, 1.00f, 0.00f,
      0.00f, 0.01f, 0.00f, 1.00f);

    // �e�p�p�C�v�������o�C���h.
    std::shared_ptr<Shader::Pipeline> pipelineShadow = GameData::Get().pipelineShadow;
    pipelineShadow->Bind();

    // �؂���񂱍s�񁨃r���[�s�񁨃v���W�F�N�V�����s��̏��ԂɊ|����s������.
    const glm::mat4 matShadowVP = matVP * matShadow;

    // �؂���񂱃r���[�v���W�F�N�V�����s����g���đS�ẴA�N�^�[��`�悷��.
    for (const auto& actor : actors) {
      actor->Draw(*pipelineShadow, matShadowVP, Actor::DrawType::shadow);
    }

    // �X�e���V���l��1�̏ꍇ�̂݃e�X�g�ɐ�������悤�ɐݒ�.
    glStencilFunc(GL_EQUAL, 1, 0xff);
    // �X�e���V���o�b�t�@
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    // �J���[�o�b�t�@�ւ̕`�����݂�����.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    // �[�x�o�b�t�@�𖳌���.
    glDisable(GL_DEPTH_TEST);

    // ��ʑS�̂ɉe�F��h��.
    texBlack->Bind(0);
    pipelineShadow->SetMVP(glm::scale(glm::mat4(1), glm::vec3(2)));
    pipelineShadow->SetMorphWeight(glm::vec3(0));
    primitiveBuffer.Get(GameData::PrimNo::plane).Draw();

    // �X�e���V���o�b�t�@�𖳌���.
    glDisable(GL_STENCIL_TEST);
    // �[�x�o�b�t�@��L����.
    glEnable(GL_DEPTH_TEST);
    // �[�x�o�b�t�@�ւ̕`�����݂�����.
    glDepthMask(GL_TRUE);
  }

  // �X�v���C�g��`��.
  spriteRenderer.Draw(pipeline, matProj * matView);

  // ���C�g�pSSBO�̃o�C���h������.
  lightManager->Unbind(0);

  // 3D���f���p��VAO���o�C���h���Ȃ����Ă���.
  primitiveBuffer.BindVertexArray();

  // �u���[���G�t�F�N�g��`��.
  {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    gamedata.samplerPostEffect.Bind(0);

    // plane���f���̃T�C�Y��0.5x0.5�Ȃ̂�1.0x1.0�ɂȂ�悤�ȍs���ݒ�.
    const glm::mat4 matMVP = glm::scale(glm::mat4(1), glm::vec3(2));

    // ���邢������fboBloom[0]�ɒ��o����.
    gamedata.pipelineHighBrightness->Bind();
    gamedata.pipelineHighBrightness->SetMVP(matMVP);
    fboBloom[0]->Bind();
    fbo->BindColorTexture(0);
    primitiveBuffer.Get(GameData::PrimNo::plane).Draw();

    // ���邢�������ڂ����Ȃ���k���R�s�[.
    gamedata.pipelineBlur->Bind();
    gamedata.pipelineBlur->SetMVP(matMVP);
    for (size_t i = 1; i < std::size(fboBloom); ++i) {
      fboBloom[i]->Bind();
      fboBloom[i - 1]->BindColorTexture(0);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }

    // �ڂ��������ʂ��g�債�Ȃ��獇�v����.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    gamedata.pipelineSimple->Bind();
    gamedata.pipelineSimple->SetMVP(matMVP);
    for (size_t i = std::size(fboBloom) - 1; i > 0; --i) {
      fboBloom[i - 1]->Bind();
      fboBloom[i]->BindColorTexture(0);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }

    // OpenGL�@�\�̐ݒ�����ɖ߂�.
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  // �`�����f�t�H���g�̃t���[���o�b�t�@�ɖ߂�.
  fbo->Unbind();

  glEnable(GL_FRAMEBUFFER_SRGB); // �K���}�␳��L���ɂ���

  // 2D�\��.
  {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    // ���W�ϊ��s����쐬.
    const float halfW = fbw / 2.0f;
    const float halfH = fbh / 2.0f;
    const glm::mat4 matProj =
      glm::ortho<float>(-halfW, halfW, -halfH, halfH, 1.0f, 500.0f);
    const glm::mat4 matView =
      glm::lookAt(glm::vec3(0, 0, 100), glm::vec3(0), glm::vec3(0, 1, 0));
    const glm::mat4 matVP = matProj * matView;

    std::shared_ptr<Shader::Pipeline> pipeline2D = GameData::Get().pipelineSimple;
    pipeline2D->Bind();

    // 3D�`�挋�ʂ�`��.
    {
      //auto pipeline = GameData::Get().pipelineSobelFilter;
      auto pipeline = GameData::Get().pipelineHatching;
      pipeline->Bind();

      glDisable(GL_BLEND);
      fbo->BindColorTexture(0);
      fbo->BindDepthStencilTexture(2);
      GameData::Get().texHatching->Bind(1);
      fboBloom[0]->BindColorTexture(3);

      const glm::mat4 matModelS = glm::scale(glm::mat4(1),
        glm::vec3(fbw, fbh, 1));
      const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
      const glm::mat4 matModel = matModelT * matModelS;
      pipeline->SetMVP(matVP * matModel);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();

      fboBloom[0]->UnbindColorTexture();
      GameData::Get().texHatching->Unbind();
      fbo->UnbindColorTexture();
      fbo->UnbindDepthStencilTexture();
      glEnable(GL_BLEND);
    }

    pipeline2D->Bind();

    // �}�E�X�J�[�\����\��.
    {
      // �}�E�X�J�[�\���摜�̃e�N�X�`���̃s�N�Z�������g�嗦�ɐݒ�.
      const glm::mat4 matModelS = glm::scale(glm::mat4(1),
        glm::vec3(texPointer->Width(), texPointer->Height(), 1));

      // �}�E�X�J�[�\�����W��\���ʒu�ɐݒ�.
      const glm::mat4 matModelT = glm::translate(glm::mat4(1),
        glm::vec3(GameData::Get().cursorPosition, 0));

      // MVP�s����v�Z���AGPU�������ɓ]��.
      const glm::mat4 matModel = matModelT * matModelS;
      pipeline2D->SetMVP(matVP * matModel);

      // �}�E�X�J�[�\���摜�̃e�N�X�`�����O���t�B�b�N�X�p�C�v���C���Ɋ��蓖�Ă�.
      texPointer->Bind(0);

      // ��̐ݒ肪�K�p���ꂽ�l�p�`��`��.
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }

    // �V���h�E�e�N�X�`���̊m�F�p.
    if (false) {
      const glm::mat4 matModelS = glm::scale(glm::mat4(1),
        glm::vec3(512, 512, 1));
      const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(-200, 100, 0));
      const glm::mat4 matModel = matModelT * matModelS;
      pipeline2D->SetMVP(matVP * matModel);
      fboShadow->BindDepthStencilTexture(0);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }

    if (false) {
      gamedata.samplerNearest.Bind(0);
      glDisable(GL_BLEND);
      glm::vec3 pos(-640, 360, 0);
      for (size_t i = 0; i < std::size(fboBloom); ++i) {
        float w = static_cast<float>(fboBloom[i]->GetWidht());
        float h = static_cast<float>(fboBloom[i]->GetHeight());
        const glm::mat4 matModelS = glm::scale(glm::mat4(1),
          glm::vec3(w, h, 1));
        pos.x = -640 + w / 2;
        pos.y -= h / 2;
        const glm::mat4 matModelT = glm::translate(glm::mat4(1), pos);
        const glm::mat4 matModel = matModelT * matModelS;
        pipeline2D->SetMVP(matVP * matModel);
        fboBloom[i]->BindColorTexture(0);
        primitiveBuffer.Get(GameData::PrimNo::plane).Draw();

        pos.y -= h - h / 2;
      }
      glEnable(GL_BLEND);
      gamedata.samplerClampToEdge.Bind(0);
    }

    // �Q�[���N���A�摜��`��.
    if (isGameClear) {
      const glm::mat4 matModelS = glm::scale(glm::mat4(1),
        glm::vec3(texGameClear->Width() * 2.0f, texGameClear->Height() * 2.0f, 1));
      const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(0, 100, 0));
      const glm::mat4 matModel = matModelT * matModelS;
      pipeline2D->SetMVP(matVP * matModel);
      texGameClear->Bind(0);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }

    // �Q�[���I�[�o�[�摜��`��.
    if (isGameOver) {
      const glm::mat4 matModelS = glm::scale(glm::mat4(1),
        glm::vec3(texGameOver->Width() * 2.0f, texGameOver->Height() * 2.0f, 1));
      const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(0, 100, 0));
      const glm::mat4 matModel = matModelT * matModelS;
      pipeline2D->SetMVP(matVP * matModel);
      texGameOver->Bind(0);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }
  }

  Texture::UnbindAllTextures();
  Texture::UnbindAllSamplers();
  Shader::UnbindPipeline();
  primitiveBuffer.UnbindVertexArray();
}

/**
* ���C���Q�[����ʂ��I������.
*/
void MainGameScene::Finalize()
{
  // �}�E�X�J�[�\����\������.
  glfwSetInputMode(GameData::Get().window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  Audio::Instance().Stop(4);

  std::cout << "[���] MainGameScene���I��.\n";
}

/**
* �V�K�A�N�^�[��ǉ�����.
* 
* @param p �ǉ�����A�N�^�[�̃|�C���^.
*/
void MainGameScene::AddActor(ActorPtr p)
{
  newActors.push_back(p);
}

/**
* �v���C���[�A�N�^�[���擾����.
*
* @return �v���C���[�A�N�^�[�ւ̃|�C���^.
*/
ActorPtr MainGameScene::GetPlayerActor()
{
  return playerActor;
}

/**
* �}�E�X�J�[�\�����W���擾����.
*
* @return �}�E�X�J�[�\�����W.
*/
const glm::vec3& MainGameScene::GetMouseCursor() const
{
  return posMouseCursor;
}

/**
* �����Ԃ��X�v���C�g��ǉ�����.
*
* @param position �X�v���C�g��\��������W.
*/
void MainGameScene::AddBloodSprite(const glm::vec3& position)
{
  GameData& gamedata = GameData::Get();
#if 0
  std::shared_ptr<Sprite> blood =
    std::make_shared<Sprite>(position + glm::vec3(0, 1, 0),gamedata.texBlood);
  const float vx = std::uniform_real_distribution<float>(-3, 3)(gamedata.random);
  const float vy = std::uniform_real_distribution<float>(-3, 3)(gamedata.random);
  const float vz = std::uniform_real_distribution<float>(-3, 3)(gamedata.random);
  blood->velocity = glm::vec3(vx, vy, vz);
  blood->scale = glm::vec2(
    std::uniform_real_distribution<float>(0.25f, 1)(gamedata.random));
  blood->gravityScale = 1;
  blood->lifespan = 0.5f;

  sprites.push_back(blood);
#elif 1
  std::shared_ptr<Sprite> blood =
    std::make_shared<Sprite>(position + glm::vec3(0, 1, 0),gamedata.texBlood);
  const float speed = std::uniform_real_distribution<float>(1.5f, 6.0f)(gamedata.random);
  const glm::mat4 matRotX = glm::rotate(glm::mat4(1),
    std::uniform_real_distribution<float>(0, glm::radians(90.0f))(gamedata.random),
    glm::vec3(1, 0, 0));
  const glm::mat4 matRotY = glm::rotate(glm::mat4(1),
    std::uniform_real_distribution<float>(0, glm::radians(360.0f))(gamedata.random),
    glm::vec3(0, 1, 0));
  blood->velocity = matRotY * matRotX * glm::vec4(0, speed, 0, 1);
  blood->rotation = std::uniform_real_distribution<float>(0, glm::radians(360.0f))(gamedata.random);
  blood->scale = glm::vec3(std::normal_distribution<float>(0, 1)(gamedata.random) * 0.25f + 0.5f);
  blood->angularVelocity= std::uniform_real_distribution<float>(glm::radians(30.0f), glm::radians(180.0f))(gamedata.random);
  blood->lifespan = std::normal_distribution<float>(0, 1)(gamedata.random) * 0.125f + 0.25f;
  blood->color = glm::vec4(1, 1, 1, 2);
  blood->colorVelocity = glm::vec4(0, 0, 0, -4.0f * (blood->lifespan / 0.5f));
  blood->scaleVelocity = glm::vec2(-0.5f);
  blood->gravityScale = 1;
  sprites.push_back(blood);
#else
  ActorPtr blood = std::make_shared<Actor>("blood",
    &gamedata.primitiveBuffer.Get(GameData::PrimNo::plane),
    gamedata.texBlood, position + glm::vec3(0, 1, 0));

  const float speed = std::uniform_real_distribution<float>(1.5f, 6.0f)(gamedata.random);
  const glm::mat4 matRotX = glm::rotate(glm::mat4(1),
    std::uniform_real_distribution<float>(0, glm::radians(90.0f))(gamedata.random),
    glm::vec3(1, 0, 0));
  const glm::mat4 matRotY = glm::rotate(glm::mat4(1),
    std::uniform_real_distribution<float>(0, glm::radians(360.0f))(gamedata.random),
    glm::vec3(0, 1, 0));
  blood->velocity = matRotY * matRotX * glm::vec4(0, speed, 0, 1);
  blood->rotation.z = std::uniform_real_distribution<float>(0, glm::radians(360.0f))(gamedata.random);
  blood->scale = glm::vec3(std::normal_distribution<float>(0, 1)(gamedata.random) * 0.5f + 0.75f);
  blood->gravityScale = 1;
  blood->lifespan = 0.5f;
  AddActor(blood);
#endif
}
