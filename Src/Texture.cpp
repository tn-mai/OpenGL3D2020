/**
* @file Texture.cpp
*/
#include "Texture.h"
#include "GLContext.h"
#include <iostream>

// �C���e���O���t�B�b�N�X�h���C�o�̃o�O�ɂ��glBindTextureUnit(i, 0)���@�\���Ȃ����Ƃւ̑Ή�.
#define AVOID_INTEL_DRIVER_BUG_FOR_GLBINDTEXTUREUNIT

/**
* �e�N�X�`���֘A�̋@�\���i�[���閼�O���.
*/
namespace Texture {

namespace /* unnamed */ {

/**
* �e�N�X�`���̃o�C���h��Ԃ�ǐՂ��邽�߂̔z��.
*
* �e�N�X�`���C���[�W���j�b�g�Ƀo�C���h���ꂽ�e�N�X�`��ID��ێ�����.
*/
GLuint textureBindingState[16] = {};

/**
* �T���v���̃o�C���h��Ԃ�ǐՂ��邽�߂̔z��.
*/
GLuint samplerBindingState[16] = {};

} // unnamed namespace

/**
* �S�Ẵe�N�X�`���̃o�C���h����������.
*/
void UnbindAllTextures()
{
  for (GLuint i = 0; i < std::size(textureBindingState); ++i) {
    textureBindingState[i] = 0;
  }
  glBindTextures(0, static_cast<GLsizei>(std::size(textureBindingState)), textureBindingState);
}

/**
* �S�ẴT���v���̃o�C���h����������.
*/
void UnbindAllSamplers()
{
  for (GLuint i = 0; i < std::size(samplerBindingState); ++i) {
    samplerBindingState[i] = 0;
  }
  glBindSamplers(0, static_cast<GLsizei>(std::size(samplerBindingState)), samplerBindingState);
}

/**
* �R���X�g���N�^.
*
* @param filename 2D�e�N�X�`���Ƃ��ēǂݍ��ރt�@�C����.
*/
Image2D::Image2D(const char* filename) : name(filename),
  id(GLContext::CreateImage2D(filename))
{
  if (id) {
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_WIDTH, &width);
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_HEIGHT, &height);
  }
}

/**
* �R���X�g���N�^.
*
* @param name    �e�N�X�`�������ʂ��邽�߂̖��O.
* @param width   �摜�̕�(�s�N�Z����).
* @param height  �摜�̍���(�s�N�Z����).
* @param data    �摜�f�[�^�ւ̃|�C���^.
* @param pixelFormat  �摜�̃s�N�Z���`��(GL_BGRA�Ȃ�).
* @param type    �摜�f�[�^�̌^.
*/
Image2D::Image2D(const char* name, GLsizei width, GLsizei height, const void* data,
  GLenum format, GLenum type) :
  name(name),
  id(GLContext::CreateImage2D(width, height, data, format, type))
{
  if (id) {
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_WIDTH, &width);
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_HEIGHT, &height);
  }
}

/**
* �f�X�g���N�^.
*/
Image2D::~Image2D()
{
  Unbind();
  glDeleteTextures(1, &id);
}

/**
* �e�N�X�`�����e�N�X�`���C���[�W���j�b�g�Ƀo�C���h����.
*
* @param unit �o�C���h��̃��j�b�g�ԍ�.
*/
void Image2D::Bind(GLuint unit) const
{
  if (unit >= std::size(textureBindingState)) {
    std::cerr << "[�G���[]" << __func__ << ": ���j�b�g�ԍ����傫�����܂�(unit=" << unit << ")\n";
    return;
  }
// �C���e���O���t�B�b�N�X�h���C�o�̃o�O�ɂ��glBindTextureUnit(i, 0)���@�\���Ȃ����Ƃւ̑Ή�.
#ifndef AVOID_INTEL_DRIVER_BUG_FOR_GLBINDTEXTUREUNIT 
  glBindTextureUnit(unit, id);
#else
  glBindTextures(unit, 1, &id);
#endif
  textureBindingState[unit] = id;
}

/**
* �e�N�X�`���̃o�C���h����������.
*/
void Image2D::Unbind() const
{
  for (GLuint i = 0; i < std::size(textureBindingState); ++i) {
    if (textureBindingState[i] == id) {
      textureBindingState[i] = 0;
// �C���e���O���t�B�b�N�X�h���C�o�̃o�O�ɂ��glBindTextureUnit(i, 0)���@�\���Ȃ����Ƃւ̑Ή�.
#ifndef AVOID_INTEL_DRIVER_BUG_FOR_GLBINDTEXTUREUNIT 
      glBindTextureUnit(i, 0);
#else
      const GLuint id = 0;
      glBindTextures(i, 1, &id);
#endif
    }
  }
}

/**
* �e�N�X�`���̕����擾����.
*
* @return �e�N�X�`���̕�(�s�N�Z����).
*/
GLsizei Image2D::Width() const
{
  return width;
}

/**
* �e�N�X�`���̍������擾����.
*
* @return �e�N�X�`���̍���(�s�N�Z����).
*/
GLsizei Image2D::Height() const
{
  return height;
}

/**
* �R���X�g���N�^.
*/
Sampler::Sampler() : id(GLContext::CreateSampler())
{
}

/**
* �f�X�g���N�^.
*/
Sampler::~Sampler()
{
  Unbind();
  glDeleteSamplers(1, &id);
}

/**
* ���b�v���[�h���w�肷��.
*
* @param mode ���b�v���[�h.
*/
void Sampler::SetWrapMode(GLenum mode)
{
  if (!id) {
    return;
  }
  glSamplerParameteri(id, GL_TEXTURE_WRAP_S, mode);
  glSamplerParameteri(id, GL_TEXTURE_WRAP_T, mode);
}

/**
* �t�B���^��ݒ肷��.
*
* @param filter �ݒ肷��t�B���^�̎��(GL_LINEAR��GL_NEAREST).
*/
void Sampler::SetFilter(GLenum filter)
{
  if (!id) {
    return;
  }
  GLenum minFilter = GL_NEAREST_MIPMAP_LINEAR;
  if (filter == GL_NEAREST) {
    minFilter = GL_NEAREST_MIPMAP_NEAREST;
  }
  glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, minFilter);
  glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, filter);
}

/**
* �T���v�����e�N�X�`���C���[�W���j�b�g�Ƀo�C���h����.
*
* @param unit �o�C���h��̃��j�b�g�ԍ�.
*/
void Sampler::Bind(GLuint unit) const
{
  if (unit >= std::size(textureBindingState)) {
    std::cerr << "[�G���[]" << __func__ << ": ���j�b�g�ԍ����傫�����܂�(unit=" << unit << ")\n";
    return;
  }
  glBindSampler(unit, id);
  samplerBindingState[unit] = id;
}

/**
* �T���v���̃o�C���h����������.
*/
void Sampler::Unbind() const
{
  for (GLuint i = 0; i < std::size(samplerBindingState); ++i) {
    if (samplerBindingState[i] == id) {
      samplerBindingState[i] = 0;
      glBindSampler(i, 0);
    }
  }
}

} // namespace Texture

