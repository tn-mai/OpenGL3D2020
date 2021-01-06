/**
* @file Shader.cpp
*/
#include "Shader.h"
#include "GLContext.h"
#include <iostream>

/**
* �V�F�[�_�Ɋւ���@�\���i�[���閼�O���.
*/
namespace Shader {

/**
* �R���X�g���N�^.
*
* @param vsCode  ���_�V�F�[�_�[�E�v���O�����̃A�h���X.
* @param fsCode  �t���O�����g�V�F�[�_�[�E�v���O�����̃A�h���X.
*/
Pipeline::Pipeline(const char* vsFilename, const char* fsFilename)
{
  vp = GLContext::CreateProgramFromFile(GL_VERTEX_SHADER, vsFilename);
  fp = GLContext::CreateProgramFromFile(GL_FRAGMENT_SHADER, fsFilename);
  id = GLContext::CreatePipeline(vp, fp);

  // �����f�[�^�]����̃v���O����ID��ݒ�.
  if (glGetUniformLocation(vp, "directionalLight.direction") >= 0) {
    lightingProgram = vp;
  } else if (glGetUniformLocation(fp, "directionalLight.direction") >= 0) {
    lightingProgram = fp;
  }

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
  glDeleteProgramPipelines(1, &id);
  glDeleteProgram(fp);
  glDeleteProgram(vp);
}

/**
* �v���O�����p�C�v���C�����o�C���h����.
*/
void Pipeline::Bind() const
{
  glBindProgramPipeline(id);
}

/**
* �v���O�����p�C�v���C���̃o�C���h����������.
*/
void Pipeline::Unbind() const
{
  glBindProgramPipeline(0);
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
bool Pipeline::SetMorphWeight(float weight) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  const GLint locMorphRatio = 10;

  glProgramUniform1f(vp, locMorphRatio, weight);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":���[�t�B���O�����䗦�̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* �v���O�����p�C�v���C���̃o�C���h����������.
*/
void UnbindPipeline()
{
  glBindProgramPipeline(0);
}

} // namespace Shader

