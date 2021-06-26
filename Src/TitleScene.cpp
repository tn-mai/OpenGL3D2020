/**
* @file TitleScene.cpp
*/
#include "TitleScene.h"
#include "SceneManager.h"
#include "GameData.h"
#include "Audio.h"
#include "Audio/MainWorkUnit/SE.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

/**
* �^�C�g����ʂ�����������.
*
* @retval true  ����������.
* @retval false ���������s.
*/
bool TitleScene::Initialize()
{
  texLogo = std::make_shared<Texture::Image2D>("Res/TitleLogo.tga");
  texPressEnter = std::make_shared<Texture::Image2D>("Res/PressEnter.tga");
  std::cout << "[���] TitleScene���J�n.\n";
  return true;
}

/**
* �^�C�g����ʂ̃L�[���͂���������.
*
* @param window GLFW�E�B���h�E�ւ̃|�C���^.
*/
void TitleScene::ProcessInput(GLFWwindow* window)
{
  GameData& gamedata = GameData::Get();
  if (gamedata.keyPressedInLastFrame & GameData::Key::enter) {
    Audio::Instance().Play(0, CRI_SE_BANG_1);
    SceneManager::Get().ChangeScene(MAINGAME_SCENE_NAME);
  }
}

/**
* �^�C�g����ʂ��X�V����.
*
* @param window    GLFW�E�B���h�E�ւ̃|�C���^.
* @param deltaTime �O��̍X�V����̌o�ߎ���.
*/
void TitleScene::Update(GLFWwindow*, float deltaTime)
{
  alpha += 0.5f * deltaTime;
  if (alpha >= 1) {
    alpha = 1;
  }
}

/**
* �^�C�g����ʂ�`�悷��.
*
* @param window GLFW�E�B���h�E�ւ̃|�C���^.
*/
void TitleScene::Render(GLFWwindow*)
{
  GameData& global = GameData::Get();
  std::shared_ptr<Shader::Pipeline> pipeline = global.pipelineSimple;
  Mesh::PrimitiveBuffer& primitiveBuffer = global.primitiveBuffer;
  Texture::Sampler& sampler = global.sampler;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_FRAMEBUFFER_SRGB); // �K���}�␳��L���ɂ���
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(0.3f, 0.2f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const glm::vec3 viewPosition(0, 0, 100);

  // ���W�ϊ��s����쐬.
  int w, h;
  glfwGetWindowSize(global.window, &w, &h);
  const glm::mat4 matProj = glm::ortho<float>(-w / 2.0f, w / 2.0f, -h / 2.0f, h / 2.0f, 1.0f, 500.0f);
  const glm::mat4 matView =
    glm::lookAt(glm::vec3(0, 0, 100), glm::vec3(0), glm::vec3(0, 1, 0));

  primitiveBuffer.BindVertexArray();
  pipeline->Bind();
  sampler.Bind(0);

  // �^�C�g�����S��`��.
  {
    const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(0, 200, 0));
    const glm::mat4 matModelS = glm::scale(glm::mat4(1), glm::vec3(texLogo->Width(), texLogo->Height(), 1));
    const glm::mat4 matMVP = matProj * matView * matModelT * matModelS;
    pipeline->SetMVP(matMVP);
    pipeline->SetObjectColor(glm::vec4(1, 1, 1, alpha));
    texLogo->Bind(0);
    global.Draw(GameData::PrimNo::plane);
  }
  {
    const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(0, -200, 0));
    const glm::mat4 matModelS = glm::scale(glm::mat4(1), glm::vec3(texPressEnter->Width(), texPressEnter->Height(), 1));
    const glm::mat4 matMVP = matProj * matView * matModelT * matModelS;
    pipeline->SetMVP(matMVP);
    pipeline->SetObjectColor(glm::vec4(1));
    texPressEnter->Bind(0);
    global.Draw(GameData::PrimNo::plane);
  }
}

/**
* �^�C�g����ʂ��I������.
*/
void TitleScene::Finalize()
{
  std::cout << "[���] TitleScene���I��.\n";
}
