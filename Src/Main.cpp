/**
* @file Main.cpp
*/
#include <glad/glad.h>
#include "GLContext.h"
#include "GameData.h"
#include "SceneManager.h"
#include "Audio.h"
#include "Audio/OpenGL3D2020_acf.h"
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

#pragma comment(lib, "opengl32.lib")

/**
* OpenGL����̃��b�Z�[�W����������.
*
* @param source    ���b�Z�[�W�̔��M��(OpenGL�AWindows�A�V�F�[�_�[�Ȃ�).
* @param type      ���b�Z�[�W�̎��(�G���[�A�x���Ȃ�).
* @param id        ���b�Z�[�W����ʂɎ��ʂ���l.
* @param severity  ���b�Z�[�W�̏d�v�x(���A���A��A�Œ�).
* @param length    ���b�Z�[�W�̕�����. �����Ȃ烁�b�Z�[�W��0�I�[����Ă���.
* @param message   ���b�Z�[�W�{��.
* @param userParam �R�[���o�b�N�ݒ莞�Ɏw�肵���|�C���^.
*
* �ڍׂ�(https://www.khronos.org/opengl/wiki/Debug_Output)���Q��.
*/
void GLAPIENTRY DebugCallback(GLenum source, GLenum type, GLuint id,
  GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
  if (length < 0) {
    std::cerr << "[MSG] " << message << "\n";
  } else {
    const std::string s(message, message + length);
    std::cerr << "[MSG] " << s << "\n";
  }
}

/**
* GLFW����̃G���[�񍐂���������.
*
* @param error �G���[�ԍ�.
* @param desc  �G���[�̓��e.
*/
void ErrorCallback(int error, const char* desc)
{
  std::cerr << "ERROR: " << desc << "\n";
}

/**
* �G���g���[�|�C���g.
*/
int main()
{
  glfwSetErrorCallback(ErrorCallback);

  // GLFW�̏�����.
  if (glfwInit() != GLFW_TRUE) {
    return 1;
  }

  // �`��E�B���h�E�̍쐬.
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  //glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
  GLFWwindow* window =
    glfwCreateWindow(1280, 720, "OpenGL3DActionGame", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);

  // OpenGL�֐��̃A�h���X���擾����.
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    std::cerr << "ERROR: GLAD�̏������Ɏ��s���܂���." << std::endl;
    glfwTerminate();
    return 1;
  }

  glDebugMessageCallback(DebugCallback, nullptr);

  // OpenGL�̏����R���\�[���E�B���h�E�֏o�͂���.
  {
#define Output(name) {\
  GLint n = 0;\
  glGetIntegerv(name, &n);\
  std::cout << #name " = " << n << "\n";\
} (void)0
    const GLubyte* renderer = glGetString(GL_RENDERER);
    std::cout << "Renderer: " << renderer << "\n";
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "OpenGL Version: " << version << "\n";
    Output(GL_MAX_VERTEX_ATTRIB_BINDINGS); // ���Ȃ��Ƃ�16.
    Output(GL_MAX_VERTEX_ATTRIBS); // ���Ȃ��Ƃ�16.
    Output(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET); // ���Ȃ��Ƃ�2047
    Output(GL_MAX_VERTEX_ATTRIB_STRIDE); // ���Ȃ��Ƃ�2048
    Output(GL_MAX_UNIFORM_LOCATIONS); // ���Ȃ��Ƃ�1024
    Output(GL_MAX_UNIFORM_BLOCK_SIZE); // ���Ȃ��Ƃ�16k
    Output(GL_MAX_UNIFORM_BUFFER_BINDINGS); // ���Ȃ��Ƃ�36
    Output(GL_MAX_VERTEX_UNIFORM_COMPONENTS); // ���Ȃ��Ƃ�1024
    Output(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS); // ���Ȃ��Ƃ�1024
    Output(GL_MAX_VARYING_COMPONENTS); // ���Ȃ��Ƃ�60
    Output(GL_MAX_VARYING_VECTORS); // ���Ȃ��Ƃ�15
    Output(GL_MAX_VARYING_FLOATS); // ���Ȃ��Ƃ�32
    Output(GL_MAX_VERTEX_ATTRIBS); // ���Ȃ��Ƃ�16
    Output(GL_MAX_TEXTURE_SIZE); // ���Ȃ��Ƃ�1024
    Output(GL_MAX_TEXTURE_IMAGE_UNITS); // ���Ȃ��Ƃ�16
    Output(GL_MAX_TEXTURE_BUFFER_SIZE); // ���Ȃ��Ƃ�64k
    Output(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS); // ���Ȃ��Ƃ�8
    Output(GL_MAX_SHADER_STORAGE_BLOCK_SIZE); // ���Ȃ��Ƃ�16MB
#undef Output
  }

  // �I�[�f�B�I������.
  Audio& audio = Audio::Instance();
  audio.Initialize("Res/Audio/OpenGL3D2020.acf", CRI_OPENGL3D2020_ACF_DSPSETTING_DSPBUSSETTING_0);
  audio.Load(0, "Res/Audio/MainWorkUnit/SE.acb", nullptr);
  audio.Load(1, "Res/Audio/MainWorkUnit/BGM.acb", "Res/Audio/MainWorkUnit/BGM.awb");

  // �Q�[���S�̂Ŏg���f�[�^������������.
  GameData& gamedata = GameData::Get();
  if (!gamedata.Initialize(window)) {
    return 1;
  }

  // �ŏ��Ɏ��s����V�[�����w�肷��.
  SceneManager& sceneManager = SceneManager::Get();
  sceneManager.ChangeScene(TITLE_SCENE_NAME);

  // �o�ߎ��Ԍv���J�n.
  double elapsedTime = glfwGetTime();

  // ���C�����[�v.
  while (!glfwWindowShouldClose(window)) {
    // �o�ߎ��Ԃ��v��.
    const double newElapsedTime = glfwGetTime();
    float deltaTime = static_cast<float>(newElapsedTime - elapsedTime);
    if (deltaTime >= 0.1f) {
      deltaTime = 1.0f / 60.0f;
    }
    elapsedTime = newElapsedTime;

    // �Q�[���f�[�^�̏�Ԃ��X�V.
    gamedata.Update();

    sceneManager.Update(window, deltaTime);
    sceneManager.Render(window);

    // �����̍X�V
    audio.Update();

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  sceneManager.Finalize();

  // �����̏I��.
  audio.Finalize();

  // GLFW�̏I��.
  glfwTerminate();

  return 0;
}