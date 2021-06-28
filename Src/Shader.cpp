/**
* @file Shader.cpp
*/
#include "Shader.h"
#include "GLContext.h"
#include <iostream>

// for Test
#include <vector>
#include <fstream>

// Radeon�Ńp�C�v���C���I�u�W�F�N�g���g����GL_COMPARE_REF_TO_TEXTURE���L���ɂȂ�Ȃ����ւ̑Ώ�.
// �v���O�����I�u�W�F�N�g���g���Ɩ��Ȃ��@�\����.
//#define AVOID_TEXTURE_COMPARE_MODE_ISSUE

/**
* �V�F�[�_�Ɋւ���@�\���i�[���閼�O���.
*/
namespace Shader {

namespace Test {

/**
* �V�F�[�_�[�E�v���O�������R���p�C������
*
* @param type �V�F�[�_�[�̎��
* @param string �V�F�[�_�[�E�v���O�����ւ̃|�C���^
*
* @retval 0 ���傫���@�쐬�����V�F�[�_�[�E�I�u�W�F�N�g
* @retval 0				�V�F�[�_�[�E�I�u�W�F�N�g�̍쐬�Ɏ��s
*/
GLuint Compile(GLenum type, const GLchar* string)
{
  if (!string) {
    return 0;
  }

  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &string, nullptr);
  glCompileShader(shader);
  GLint compiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  // �R���p�C���Ɏ��s�����ꍇ�A�������R���\�[���ɏo�͂��ĂO��Ԃ�
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen) {
      std::vector<char> buf;
      buf.resize(infoLen);
      if (static_cast<int>(buf.size()) >= infoLen) {
        glGetShaderInfoLog(shader, infoLen, NULL, buf.data());
        std::cerr << "ERROR: �V�F�[�_�[�̃R���p�C���Ɏ��s.\n" << buf.data() << std::endl;
      }
    }
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

/**
* �v���O�����E�I�u�W�F�N�g���쐬����
*
* @param vsCode ���_�V�F�[�_�[�E�v���O�����ւ̃|�C���^
* @param fsCode �t���O�����g�V�F�[�_�[�E�v���O�����ւ̃|�C���^
*
* @retval 0���傫�� �쐬�����v���O�����E�I�u�W�F�N�g
* @retval 0				�v���O�����E�I�u�W�F�N�g�̍쐬�Ɏ��s
*/
GLuint Build(const GLchar* vsCode, const GLchar* fsCode)
{
  GLuint vs = Compile(GL_VERTEX_SHADER, vsCode);
  GLuint fs = Compile(GL_FRAGMENT_SHADER, fsCode);
  if (!vs || !fs) {
    return 0;
  }
  GLuint program = glCreateProgram();
  glAttachShader(program, fs);
  glDeleteShader(fs);
  glAttachShader(program, vs);
  glDeleteShader(vs);
  glLinkProgram(program);
  GLint linkStatus = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
  if (linkStatus != GL_TRUE) {
    GLint infoLen = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen) {
      std::vector<char> buf;
      buf.resize(infoLen);
      if (static_cast<int>(buf.size()) >= infoLen) {
        glGetProgramInfoLog(program, infoLen, NULL, buf.data());
        std::cerr << "ERROR: �V�F�[�_�[�����N�Ɏ��s.\n" << buf.data() << std::endl;
      }
    }
    glDeleteProgram(program);
    return 0;
  }
  return program;
}

/**
* �t�@�C����ǂݍ���
*
* @param path �ǂݍ��ރt�@�C����
*
* @return �ǂݍ��񂾃f�[�^
*/
std::vector<GLchar> ReadFile(const char* path)
{
  std::basic_ifstream<GLchar> ifs;
  ifs.open(path, std::ios_base::binary);
  if (!ifs.is_open()) {
    std::cerr << "ERROR: " << path << " ���J���܂���.\n";
    return{};
  }
  ifs.seekg(0, std::ios_base::end);
  const size_t length = (size_t)ifs.tellg();
  ifs.seekg(0, std::ios_base::beg);
  std::vector<GLchar> buf(length);
  ifs.read(buf.data(), length);
  buf.push_back('\0');
  return buf;
}
/**
* �t�@�C������v���O�����E�I�u�W�F�N�g���쐬����
*
* @param vsPath ���_�V�F�[�_�[�E�t�@�C����
* @param fsPath �t���O�����g�V�F�[�_�[�E�t�@�C����
*
* @return �쐬�����v���O�����E�I�u�W�F�N�g
*/
GLuint BuildFromFile(const char* vsPath, const char* fsPath)
{
  const std::vector<GLchar> vsCode = ReadFile(vsPath);
  const std::vector<GLchar> fsCode = ReadFile(fsPath);
  return Build(vsCode.data(), fsCode.data());
}

} // namespace Test

/**
* �R���X�g���N�^.
*
* @param vsCode  ���_�V�F�[�_�[�E�v���O�����̃A�h���X.
* @param fsCode  �t���O�����g�V�F�[�_�[�E�v���O�����̃A�h���X.
*/
Pipeline::Pipeline(const char* vsFilename, const char* fsFilename)
{
#ifndef AVOID_TEXTURE_COMPARE_MODE_ISSUE
  vp = GLContext::CreateProgramFromFile(GL_VERTEX_SHADER, vsFilename);
  fp = GLContext::CreateProgramFromFile(GL_FRAGMENT_SHADER, fsFilename);
  id = GLContext::CreatePipeline(vp, fp);

  // �����f�[�^�]����̃v���O����ID��ݒ�.
  if (glGetUniformLocation(vp, "directionalLight.direction") >= 0) {
    lightingProgram = vp;
  } else if (glGetUniformLocation(fp, "directionalLight.direction") >= 0) {
    lightingProgram = fp;
  }
#else
  id = vp = fp = Test::BuildFromFile(vsFilename, fsFilename);
  lightingProgram = vp;
#endif
  // �I�u�W�F�N�g�J���[�̏����l��ݒ�.
  if (glGetUniformLocation(vp, "objectColor") >= 0) {
    SetObjectColor(glm::vec4(1));
  }
}

/**
* �f�X�g���N�^.
*/
Pipeline::~Pipeline()
{
#ifndef AVOID_TEXTURE_COMPARE_MODE_ISSUE
  glDeleteProgramPipelines(1, &id);
  glDeleteProgram(fp);
#endif
  glDeleteProgram(vp);
}

/**
* �v���O�����p�C�v���C�����o�C���h����.
*/
void Pipeline::Bind() const
{
#ifndef AVOID_TEXTURE_COMPARE_MODE_ISSUE
  glUseProgram(0);
  glBindProgramPipeline(id);
#else
  glUseProgram(vp);
#endif
}

/**
* �v���O�����p�C�v���C���̃o�C���h����������.
*/
void Pipeline::Unbind() const
{
#ifndef AVOID_TEXTURE_COMPARE_MODE_ISSUE
  glBindProgramPipeline(0);
#else
  glUseProgram(0);
#endif
}

/**
* �V�F�[�_��MVP�s���ݒ肷��.
*
* @param matMVP �ݒ肷��MVP�s��.
*
* @retval true  �ݒ萬��.
* @retval false �ݒ莸�s.
*/
bool Pipeline::SetMVP(const glm::mat4& matMVP) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  const GLint locMatMVP = 0;
  glProgramUniformMatrix4fv(vp, locMatMVP, 1, GL_FALSE, &matMVP[0][0]);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":MVP�s��̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* �V�F�[�_�Ƀ��f���s���ݒ肷��.
*
* @param matModel �ݒ肷��@���s��.
*
* @retval true  �ݒ萬��.
* @retval false �ݒ莸�s.
*/
bool Pipeline::SetModelMatrix(const glm::mat4& matModel) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  const GLint locMatModel = 1;
  glProgramUniformMatrix4fv(vp, locMatModel, 1, GL_FALSE, &matModel[0][0]);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":���f���s��̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* �V�F�[�_�ɃV���h�E�s���ݒ肷��.
*
* @param matShadow �ݒ肷��V���h�E�s��.
*
* @retval true  �ݒ萬��.
* @retval false �ݒ莸�s.
*/
bool Pipeline::SetShadowMatrix(const glm::mat4& matShadow) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  const GLint locMatShadow = 11;
  glProgramUniformMatrix4fv(vp, locMatShadow, 1, GL_FALSE, &matShadow[0][0]);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":�V���h�E�s��̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* �V�F�[�_�Ƀ��C�g�f�[�^��ݒ肷��.
*
* @param light �ݒ肷�郉�C�g�f�[�^.
*
* @retval true  �ݒ萬��.
* @retval false �ݒ莸�s.
*/
bool Pipeline::SetLight(const DirectionalLight& light) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  const GLint locDirLight = 2;

  glProgramUniform4fv(lightingProgram, locDirLight, 1, &light.direction.x);
  glProgramUniform4fv(lightingProgram, locDirLight + 1, 1, &light.color.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":���s�����̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* �V�F�[�_�Ƀ��C�g�f�[�^��ݒ肷��.
*
* @param light �ݒ肷�郉�C�g�f�[�^.
*
* @retval true  �ݒ萬��.
* @retval false �ݒ莸�s.
*/
bool Pipeline::SetLight(const PointLight& light) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  const GLint locPointLight = 4;

  glProgramUniform4fv(lightingProgram, locPointLight, 1, &light.position.x);
  glProgramUniform4fv(lightingProgram, locPointLight + 1, 1, &light.color.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":�_�����̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* �V�F�[�_�Ɋ�����ݒ肷��.
*
* @param color �ݒ肷�����.
*
* @retval true  �ݒ萬��.
* @retval false �ݒ莸�s.
*/
bool Pipeline::SetAmbientLight(const glm::vec3& color) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  const GLint locAmbientLight= 6;

  glProgramUniform3fv(lightingProgram, locAmbientLight, 1, &color.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":�����̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* �V�F�[�_�ɃI�u�W�F�N�g�̐F��ݒ肷��.
*
* @param color �ݒ肷��F.
*
* @retval true  �ݒ萬��.
* @retval false �ݒ莸�s.
*/
bool Pipeline::SetObjectColor(const glm::vec4& color) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  const GLint locObjectColor = 8;

  glProgramUniform4fv(vp, locObjectColor, 1, &color.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":�I�u�W�F�N�g�J���[�̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* ���[�t�B���O�̍����䗦��ݒ肷��.
*
* @param weight ���[�t�B���O�̍����䗦(0.0�`1.0�͈̔͂Ŏw�肷�邱��).
*/
bool Pipeline::SetMorphWeight(const glm::vec3& weight) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  const GLint locMorphRatio = 10;

  glProgramUniform3f(vp, locMorphRatio, weight.x, weight.y, weight.z);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":���[�t�B���O�����䗦�̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* �V�F�[�_�Ɏ��_���W��ݒ肷��.
*
* @param position �ݒ肷�鎋�_���W.
*
* @retval true  �ݒ萬��.
* @retval false �ݒ莸�s.
*/
bool Pipeline::SetViewPosition(const glm::vec3& position) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  const GLint locViewPosition = 7;
  glProgramUniform3f(fp, locViewPosition, position.x, position.y, position.z);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":���_���W�̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* �V�F�[�_�Ƀ^�C�}�[�l��ݒ肷��.
*
* @param time �ݒ肷��^�C�}�[�l.
*
* @retval true  �ݒ萬��.
* @retval false �ݒ莸�s.
*/
bool Pipeline::SetEffectTimer(float time) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  const GLint locTime = 9;
  glProgramUniform1f(fp, locTime, time);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":�^�C�}�[�l�̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}
/**
* �v���O�����p�C�v���C���̃o�C���h����������.
*/
void UnbindPipeline()
{
#ifndef AVOID_TEXTURE_COMPARE_MODE_ISSUE
  glBindProgramPipeline(0);
#else
  glUseProgram(0);
#endif
}

} // namespace Shader

