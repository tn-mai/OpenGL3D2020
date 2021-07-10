/**
* @file Texture.cpp
*/
#include "Texture.h"
#include "GLContext.h"
#include <unordered_map>
#include <iostream>

// �C���e���O���t�B�b�N�X�h���C�o�̃o�O�ɂ��glBindTextureUnit(i, 0)���@�\���Ȃ����Ƃւ̑Ή�.
#define AVOID_INTEL_DRIVER_BUG_FOR_GLBINDTEXTUREUNIT

// AMD�h���C�o�̃o�O�ɂ��glBindTextures, glBindTextureUnit���g���ƃo�C���h�������x��������ւ̑Ή�.
// glBindTextures, glBindTextureUnit�ɂ���ăe�N�X�`�����o�C���h�����Ƃ��A
// �o�C���h������glBindTextures, glBindTextureUnit���g���ƒ���̕`��ł̓o�C���h����������Ȃ����Ƃ�����.
// �����e�N�X�`�����u�o�C���h->�A���o�C���h�v����ꍇ�͖��Ȃ����A�u�o�C���h*n->�A���o�C���h�v�Ƃ���ƒx������������.
// �Q�ƃJ�E���g���^�������u�o�C���h*n->�A���o�C���h*n�v�ł��x������̂ŁA�����łȂ��\��������.
// �������A�A���o�C���h��glActiveTexture&glBindTexture���g���ƒx�������A����̕`��R�}���h�ł��������A���o�C���h����Ă���.
// �܂��A�o�C���h��glActiveTexture&glBindTexture���g���ƁA�ǂ̕��@�ł�1��Ő������A���o�C���h�����.
// �o�C���h��`glActiveTexture&glBindTexture`���g���Ă����΃A���o�C���h�͂ǂ̕��@�ł��@�\����悤�Ɍ����邪�A
// �����ȂƂ���A�o�C���h�ƃA���o�C���h�ňقȂ�֐����g�������ʁA�܂��o�O�𓥂ނ������ꂸ�s���͎c��.
// ���ǁA"AMD�h���C�o�ł�glBindTextures, glBindTextureUnit���g����"�Ƃ������ƂɂȂ�.
// �Ȃ�AMD�͊���OpenGL�h���C�o�̍X�V���I������Vulkan�Ɉڍs���Ă���. ���̖�肪���P�������͗��Ȃ����낤.
#define AVOID_AMD_DRIVER_BUG_FOR_GLBINDTEXTURES

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

/**
* �e�N�X�`���E�L���b�V��.
*/
std::unordered_map<std::string, std::shared_ptr<Image2D>> textureCache;

} // unnamed namespace

/**
* �e�N�X�`���̃o�C���h����������.
*
* @param unit ��������o�C���f�B���O�|�C���g.
*/
void UnbindTexture(GLuint unit)
{
#ifndef AVOID_INTEL_DRIVER_BUG_FOR_GLBINDTEXTUREUNIT 
  glBindTextureUnit(unit, 0);
#else
# ifndef AVOID_AMD_DRIVER_BUG_FOR_GLBINDTEXTURES
  glBindTextures(unit, 1, nullptr);
# else
  static const GLenum targets[] = {
    GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D,
    GL_TEXTURE_1D_ARRAY, GL_TEXTURE_2D_ARRAY,
    GL_TEXTURE_RECTANGLE,
    GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_ARRAY,
    GL_TEXTURE_BUFFER,
    GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_2D_MULTISAMPLE_ARRAY
  };
  glActiveTexture(GL_TEXTURE0 + unit);
  for (auto e : targets) {
    glBindTexture(e, 0);
  }
  glActiveTexture(GL_TEXTURE0);
# endif
#endif
  textureBindingState[unit] = 0;
}

/**
* �S�Ẵe�N�X�`���̃o�C���h����������.
*/
void UnbindAllTextures()
{
#ifndef AVOID_AMD_DRIVER_BUG_FOR_GLBINDTEXTURES
  for (GLuint i = 0; i < std::size(textureBindingState); ++i) {
    textureBindingState[i] = 0;
  }
  glBindTextures(0, static_cast<GLsizei>(std::size(textureBindingState)), textureBindingState);
#else
  for (GLuint i = 0; i < std::size(textureBindingState); ++i) {
    UnbindTexture(i);
  }
#endif
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
* 2D�e�N�X�`�����쐬����.
*
* @param filename 2D�e�N�X�`���Ƃ��ēǂݍ��ރt�@�C����.
* @param imageType �摜�̎��.
* 
* @return �쐬�����e�N�X�`��.
*/
std::shared_ptr<Image2D> CreateImage2D(const char* filename, ImageType imageType)
{
  // �L���b�V�����������A�����̃e�N�X�`������������A�������e�N�X�`����Ԃ�.
  auto itr = textureCache.find(filename);
  if (itr != textureCache.end()) {
    return itr->second;
  }

  // �L���b�V���ɓ����̃e�N�X�`�����Ȃ������̂ŁA�V�����e�N�X�`�������.
  // �쐬�����e�N�X�`�����e�N�X�`���L���b�V���ɒǉ����A����ɖ߂�l�Ƃ��ĕԂ�.
  std::shared_ptr<Image2D> p = std::make_shared<Image2D>(filename, imageType);
  textureCache.emplace(filename, p);
  return p;
}

/**
* �e�N�X�`���L���b�V������ɂ���.
*/
void ClearTextureCache()
{
  textureCache.clear();
}

/**
* �摜�̎�ނɑΉ�����GPU���̃s�N�Z���`�����擾����.
*
* @param imageType �摜�̎��.
*
* @return type�ɑΉ�����GPU���̃s�N�Z���`��.
*/
GLenum ToInternalFormat(ImageType imageType)
{
  switch (imageType) {
  default:
  case ImageType::framebuffer: return GL_RGBA16F;
  case ImageType::depthbuffer: return GL_DEPTH24_STENCIL8;
  case ImageType::color:       return GL_SRGB8_ALPHA8;
  case ImageType::non_color:   return GL_RGBA8;
  }
}

/**
* �R���X�g���N�^.
*
* @param filename 2D�e�N�X�`���Ƃ��ēǂݍ��ރt�@�C����.
* @param imageType �摜�̎��.
*/
Image2D::Image2D(const char* filename, ImageType imageType) :
  name(filename),
  id(GLContext::CreateImage2D(filename, ToInternalFormat(imageType)))
{
  if (id) {
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_WIDTH, &width);
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_HEIGHT, &height);
  }
  if (id) {
    GLint internalFormat;
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
    const char* ifname;
    switch (internalFormat) {
    default: ifname = "(unknown)"; break;
    case GL_RGBA8: ifname = "GL_RGBA8"; break;
    case GL_SRGB8_ALPHA8: ifname = "GL_SRGB8_ALPHA8"; break;
    }
    std::cout << "[���]" << __func__ << "�e�N�X�`��" << filename << "���쐬(" <<
      width << "x" << height << " " << ifname << ").\n";
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
* @param imageType �摜�̎��.
*/
Image2D::Image2D(const char* name, GLsizei width, GLsizei height, const void* data,
  GLenum pixelFormat, GLenum type, ImageType imageType) :
  name(name),
  id(GLContext::CreateImage2D(width, height, data, pixelFormat, type,
    ToInternalFormat(imageType)))
{
  if (id) {
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_WIDTH, &this->width);
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_HEIGHT, &this->height);
  }
  
  if (id) {
    const char* pfname;
    switch (pixelFormat) {
    default: pfname = "GL_BGRA"; break;
    case GL_BGR: pfname = "GL_BGR"; break;
    case GL_RED: pfname = "GL_RED"; break;
    }
    const char* ifname;
    switch (imageType) {
    default: ifname = "(unknown)"; break;
    case ImageType::framebuffer: ifname = "GL_RGBA8"; break;
    case ImageType::depthbuffer: ifname = "GL_DEPTH24_STENCIL8"; break;
    case ImageType::color:       ifname = "GL_SRGB8_ALPHA8"; break;
    case ImageType::non_color:   ifname = "GL_RGBA8"; break;
    }
    std::cout << "[���]" << __func__ << "�e�N�X�`��" << name << "���쐬(" <<
      width << "x" << height << " " << pfname << " " << ifname << ").\n";
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
#ifndef AVOID_INTEL_DRIVER_BUG_FOR_GLBINDTEXTUREUNIT 
  glBindTextureUnit(unit, id);
#else
# ifndef AVOID_AMD_DRIVER_BUG_FOR_GLBINDTEXTURES
  glBindTextures(unit, 1, &id);
# else
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, id);
  glActiveTexture(GL_TEXTURE0);
# endif
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
      UnbindTexture(i);
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
  GLenum minFilter = GL_LINEAR_MIPMAP_NEAREST;
  if (filter == GL_NEAREST) {
    minFilter = GL_NEAREST_MIPMAP_NEAREST;
  }
  glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, minFilter);
  glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, filter);
}

/**
* �f�v�X�V���h�E�}�b�v�p�̐[�x��r���[�h��ݒ肷��.
*
* @param compareFunc �ݒ肷��[�x��r���[�h(GL_LEQUAL�Ȃ�).
*                    �[�x��r���[�h�𖳌�������ɂ�GL_NONE���w�肷��.
*/
void Sampler::SetShadowMode(GLenum compareFunc)
{
  if (!id) {
    return;
  }
  if (compareFunc == GL_NONE) {
    glSamplerParameteri(id, GL_TEXTURE_COMPARE_MODE, GL_NONE);
  } else {
    glSamplerParameteri(id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glSamplerParameteri(id, GL_TEXTURE_COMPARE_FUNC, compareFunc);
  }
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

