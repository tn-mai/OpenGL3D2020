/**
* @file GameData.cpp
*/
#include "GameData.h"
#include <iostream>

/**
*
*/
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  GameData::Get().tmpScroll += yoffset;
}

/**
*
*/
GameData& GameData::Get()
{
  static GameData singleton;
  return singleton;
}

/**
* �f�X�g���N�^.
*/
GameData::~GameData()
{
  std::cout << "[���] �Q�[���f�[�^��j��.\n";
}

/**
* �O���[�o���f�[�^������������.
*
* @param window GLFW�E�B���h�E�ւ̃|�C���^.
*
* @retval true  ����������.
* @retval false ���������s.
*/
bool GameData::Initialize(GLFWwindow* window)
{
  std::cout << "[���] �Q�[���f�[�^�̏��������J�n.\n";

  glfwSetScrollCallback(window, ScrollCallback);

  // �v���~�e�B�u�o�b�t�@�Ƀ��f���f�[�^��ǂݍ���.
  if (!primitiveBuffer.Allocate(200'000, 800'000)) {
    return false;
  }
  primitiveBuffer.AddFromObjFile("Res/Ground.obj");
  primitiveBuffer.AddFromObjFile("Res/Tree.obj");
  primitiveBuffer.AddFromObjFile("Res/House.obj");
  primitiveBuffer.AddFromObjFile("Res/Cube.obj");
  primitiveBuffer.AddFromObjFile("Res/Plane.obj");
  primitiveBuffer.AddFromObjFile("Res/Bullet.obj");
  primitiveBuffer.AddFromObjFile("Res/m67_grenade.obj");
  primitiveBuffer.AddFromObjFile("Res/wooden_barrier.obj");

  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_0.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_1.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_2.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_3.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_4.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_5.obj");

  primitiveBuffer.AddFromObjFile("Res/zombie_male_down_0.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_down_1.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_down_2.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_down_3.obj");

  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_attack_0.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_attack_1.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_attack_2.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_attack_3.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_attack_4.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_attack_5.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_attack_6.obj");

  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_damage_0.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_damage_1.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_damage_2.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_damage_3.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male_idle_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_idle_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_idle_2.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male_run_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_2.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_3.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_4.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_5.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_back_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_back_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_back_2.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_back_3.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_back_4.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_back_5.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_left_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_left_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_left_2.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_left_3.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_left_4.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_left_5.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_right_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_right_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_right_2.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_right_3.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_right_4.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_right_5.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_down_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_down_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_down_2.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_down_3.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_damage_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_damage_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_damage_2.obj");

  // �p�C�v���C���E�I�u�W�F�N�g���쐬����.
  pipeline = std::make_shared<Shader::Pipeline>("Res/FragmentLighting.vert", "Res/FragmentLighting.frag");
  if (!pipeline || !*pipeline) {
    return false;
  }
  pipelineSimple = std::make_shared<Shader::Pipeline>("Res/Simple.vert", "Res/Simple.frag");
  if (!pipelineSimple || !*pipelineSimple) {
    return false;
  }
  pipelineShadow = std::make_shared<Shader::Pipeline>("Res/Shadow.vert", "Res/Shadow.frag");
  if (!pipelineShadow || !*pipelineShadow) {
    return false;
  }
  pipelineSobelFilter = std::make_shared<Shader::Pipeline>("Res/Simple.vert", "Res/SobelFilter.frag");
  if (!pipelineSobelFilter || !*pipelineSobelFilter) {
    return false;
  }
  pipelineGaussianFilter = std::make_shared<Shader::Pipeline>("Res/Simple.vert", "Res/Filter_GaussianBlur.frag");
  if (!pipelineGaussianFilter || !*pipelineGaussianFilter) {
    return false;
  }
  pipelinePosterization = std::make_shared<Shader::Pipeline>("Res/Simple.vert", "Res/Filter_Posterization.frag");
  if (!pipelinePosterization || !*pipelinePosterization) {
    return false;
  }
  pipelineHatching = std::make_shared<Shader::Pipeline>("Res/Simple.vert", "Res/Filter_Hatching.frag");
  if (!pipelineHatching || !*pipelineHatching) {
    return false;
  }
  pipelineOutline = std::make_shared<Shader::Pipeline>("Res/Simple.vert", "Res/Filter_Outline.frag");
  if (!pipelineOutline || !*pipelineOutline) {
    return false;
  }

  // �T���v���E�I�u�W�F�N�g���쐬����.
  sampler.SetWrapMode(GL_REPEAT);
  sampler.SetFilter(GL_NEAREST);

  for (auto& e : samplers) {
    e.SetWrapMode(GL_REPEAT);
    e.SetFilter(GL_LINEAR);
  }
  samplers[1].SetWrapMode(GL_CLAMP_TO_EDGE);
  //samplers[1].SetFilter(GL_NEAREST);

  this->window = window;

  random.seed(std::random_device{}());

  /* �A�j���[�V�����f�[�^���쐬 */

  anmZombieMaleWalk = std::make_shared<Animation>();
  anmZombieMaleWalk->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_walk_0));
  anmZombieMaleWalk->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_walk_1));
  anmZombieMaleWalk->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_walk_2));
  anmZombieMaleWalk->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_walk_3));
  anmZombieMaleWalk->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_walk_4));
  anmZombieMaleWalk->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_walk_5));
  anmZombieMaleWalk->interval = 0.2f;

  anmZombieMaleDown = std::make_shared<Animation>();
  anmZombieMaleDown->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_down_0));
  anmZombieMaleDown->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_down_1));
  anmZombieMaleDown->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_down_2));
  anmZombieMaleDown->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_down_3));
  anmZombieMaleDown->interval = 0.125f;
  anmZombieMaleDown->isLoop = false;

  anmZombieMaleAttack = std::make_shared<Animation>();
  anmZombieMaleAttack->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_attack_0));
  anmZombieMaleAttack->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_attack_1));
  anmZombieMaleAttack->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_attack_2));
  anmZombieMaleAttack->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_attack_3));
  anmZombieMaleAttack->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_attack_4));
  anmZombieMaleAttack->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_attack_5));
  anmZombieMaleAttack->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_attack_6));
  anmZombieMaleAttack->interval = 0.125f;
  anmZombieMaleAttack->isLoop = false;

  anmZombieMaleDamage = std::make_shared<Animation>();
  anmZombieMaleDamage->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_damage_0));
  anmZombieMaleDamage->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_damage_1));
  anmZombieMaleDamage->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_damage_2));
  anmZombieMaleDamage->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_damage_3));
  anmZombieMaleDamage->interval = 0.1f;
  anmZombieMaleDamage->isLoop = false;


  anmPlayerIdle = std::make_shared<Animation>();
  anmPlayerIdle->list.push_back(&primitiveBuffer.Get(PrimNo::player_idle_0));
  anmPlayerIdle->list.push_back(&primitiveBuffer.Get(PrimNo::player_idle_1));
  anmPlayerIdle->list.push_back(&primitiveBuffer.Get(PrimNo::player_idle_2));
  anmPlayerIdle->list.push_back(&primitiveBuffer.Get(PrimNo::player_idle_1));
  anmPlayerIdle->interval = 0.2f;

  anmPlayerRunFront = std::make_shared<Animation>();
  anmPlayerRunFront->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_front_0));
  anmPlayerRunFront->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_front_1));
  anmPlayerRunFront->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_front_2));
  anmPlayerRunFront->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_front_3));
  anmPlayerRunFront->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_front_4));
  anmPlayerRunFront->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_front_5));
  anmPlayerRunFront->interval = 0.125f;

  anmPlayerRunBack = std::make_shared<Animation>();
  anmPlayerRunBack->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_back_0));
  anmPlayerRunBack->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_back_1));
  anmPlayerRunBack->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_back_2));
  anmPlayerRunBack->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_back_3));
  anmPlayerRunBack->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_back_4));
  anmPlayerRunBack->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_back_5));
  anmPlayerRunBack->interval = 0.125f;

  anmPlayerRunLeft = std::make_shared<Animation>();
  anmPlayerRunLeft->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_left_0));
  anmPlayerRunLeft->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_left_1));
  anmPlayerRunLeft->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_left_2));
  anmPlayerRunLeft->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_left_3));
  anmPlayerRunLeft->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_left_4));
  anmPlayerRunLeft->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_left_5));
  anmPlayerRunLeft->interval = 0.125f;

  anmPlayerRunRight = std::make_shared<Animation>();
  anmPlayerRunRight->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_right_0));
  anmPlayerRunRight->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_right_1));
  anmPlayerRunRight->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_right_2));
  anmPlayerRunRight->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_right_3));
  anmPlayerRunRight->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_right_4));
  anmPlayerRunRight->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_right_5));
  anmPlayerRunRight->interval = 0.125f;

  anmPlayerDown = std::make_shared<Animation>();
  anmPlayerDown->list.push_back(&primitiveBuffer.Get(PrimNo::player_down_0));
  anmPlayerDown->list.push_back(&primitiveBuffer.Get(PrimNo::player_down_1));
  anmPlayerDown->list.push_back(&primitiveBuffer.Get(PrimNo::player_down_2));
  anmPlayerDown->list.push_back(&primitiveBuffer.Get(PrimNo::player_down_3));
  anmPlayerDown->interval = 0.2f;
  anmPlayerDown->isLoop = false;

  anmPlayerDamage = std::make_shared<Animation>();
  anmPlayerDamage->list.push_back(&primitiveBuffer.Get(PrimNo::player_damage_1));
  anmPlayerDamage->list.push_back(&primitiveBuffer.Get(PrimNo::player_damage_2));
  anmPlayerDamage->list.push_back(&primitiveBuffer.Get(PrimNo::player_damage_1));
  anmPlayerDamage->list.push_back(&primitiveBuffer.Get(PrimNo::player_damage_0));
  anmPlayerDamage->interval = 0.1f;
  anmPlayerDamage->isLoop = false;

  texBlood = std::make_shared<Texture::Image2D>("Res/blood.tga");
  texHatching = std::make_shared<Texture::Image2D>("Res/Hatching.tga");

  std::cout << "[���] �Q�[���f�[�^�̏�����������.\n";
  return true;
}

/**
* �Q�[���f�[�^�̍X�V.
*/
void GameData::Update()
{
  // [�L�[���̍X�V]
  {
    // �Q�[���f�[�^�̃L�[�ԍ���GLFW�̃L�[�ԍ��̑Ή��\�����.
    // ����L�[�𑝂₵����ς������Ƃ��͂��̑Ή��\��ύX����.
    const struct {
      Key keyGamedata;  // �Q�[���f�[�^�̃L�[.
      uint32_t keyGlfw; // GLFW�̃L�[.
    } keyMap[] = {
      { Key::enter, GLFW_KEY_ENTER },
      { Key::left,  GLFW_KEY_A },
      { Key::right, GLFW_KEY_D },
      { Key::up,    GLFW_KEY_W },
      { Key::down,  GLFW_KEY_S },
      { Key::jump,  GLFW_KEY_SPACE },
    };

    // ���݉�����Ă���L�[���擾.
    uint32_t newKey = 0; // ���݉�����Ă���L�[.
    for (const auto& e : keyMap) {
      if (glfwGetKey(window, e.keyGlfw) == GLFW_PRESS) {
        newKey |= e.keyGamedata;
      }
    }
    // �}�E�X�̍��{�^���Ŏˌ�.
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
      newKey |= Key::shot;
    }
    // �}�E�X�̉E�{�^���Ŏ�֒e�𓊂���.
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
      newKey |= Key::grenade;
    }

    // �X�N���[���̍X�V.
    prevScroll = curScroll;
    curScroll = tmpScroll;
    const double scroll = curScroll - prevScroll;
    if (scroll <= -1) {
      newKey |= Key::scrollup;
    } else if (scroll >= 1) {
      newKey |= Key::scrolldown;
    }

    // �O���Update�ŉ�����Ă��炸(~keyPressed)�A
    // �����݉�����Ă���(newKey)�L�[���Ō�̃t���[���ŉ����ꂽ�L�[�ɐݒ�.
    keyPressedInLastFrame = ~keyPressed & newKey;

    // ������Ă���L�[���X�V.
    keyPressed = newKey;
  }

  // �}�E�X�J�[�\�����W�̍X�V.
  {
    // �}�E�X�J�[�\�����W��ϐ�x��y�Ɏ擾.
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    /* �擾�������W��OpenGL���W�n�ɕϊ�. */

    // �E�B���h�E�T�C�Y��ϐ�w��h�Ɏ擾.
    int w, h;
    glfwGetWindowSize(window, &w, &h);

    // �u�������_�A�オ+Y�v�̍��W�n�ɕϊ�.
    // 1�������Ă���̂́A�Ⴆ�΍���720�̏ꍇ�A���W����肤��͈͂�0�`719��720�i�K�ɂȂ邽�߁B
    y = (h - 1.0) - y;

    // �u��ʒ��S�����_�A�オ+Y�v�̍��W�n(�܂�OpenGL�̍��W�n)�ɕϊ�.
    x -= w * 0.5;
    y -= h * 0.5;

    // ���W��float�^�ɕϊ�����cursorPosition�����o�ϐ��ɑ��.
    // (OpenGL�͊�{�I��float�^�ŏ������s���̂ŁA�^�����킹�Ă����ƈ����₷��).
    cursorPosition.x = static_cast<float>(x);
    cursorPosition.y = static_cast<float>(y);
  }
}

/**
* �v���~�e�B�u��`�悷��.
*
* @param id �v���~�e�B�u��ID.
*/
void GameData::Draw(GameData::PrimNo id) const
{
  primitiveBuffer.Get(static_cast<size_t>(id)).Draw();
}
