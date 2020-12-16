/**
* @file FramebufferObject.cpp
*/
#include "FramebufferObject.h"
#include "GameData.h"
#include <iostream>

/**
* �R���X�g���N�^.
*/
FramebufferObject::FramebufferObject(int w, int h, FboType type)
{
  texColor = std::make_shared<Texture::Image2D>("FBO(Color)", w, h, nullptr, GL_RGBA, GL_UNSIGNED_BYTE);
  if (!texColor || !texColor->GetId()) {
    std::cerr << "[�G���[]" << __func__ << ":�I�t�X�N���[���o�b�t�@�p�e�N�X�`���̍쐬�Ɏ��s.\n";
    texColor.reset();
    return;
  }

  if (type == FboType::Color) {
    glCreateRenderbuffers(1, &depthStencil);
    glNamedRenderbufferStorage(depthStencil, GL_DEPTH24_STENCIL8, w, h);

    glCreateFramebuffers(1, &fbo);
    glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, texColor->GetId(), 0);
    glNamedFramebufferRenderbuffer(fbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencil);
  } else {
    texDepthStencil = std::make_shared<Texture::Image2D>("FBO(DepthStencil)", w, h, nullptr, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
    if (!texDepthStencil || !texDepthStencil->GetId()) {
      std::cerr << "[�G���[]" << __func__ << ":�I�t�X�N���[���o�b�t�@�p�e�N�X�`���̍쐬�Ɏ��s.\n";
      texColor.reset();
      texDepthStencil.reset();
      return;
    }

    glCreateFramebuffers(1, &fbo);
    glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, texColor->GetId(), 0);
    glNamedFramebufferTexture(fbo, GL_DEPTH_STENCIL_ATTACHMENT, texDepthStencil->GetId(), 0);
  }

  if (glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "[�G���[]" << __func__ << ":�I�t�X�N���[���o�b�t�@�̍쐬�Ɏ��s.\n";
    glDeleteFramebuffers(1, &fbo);
    fbo = 0;
    texColor.reset();
    texDepthStencil.reset();
    return;
  }

  width = w;
  height = h;
}

/**
* �f�X�g���N�^.
*/
FramebufferObject::~FramebufferObject()
{
  glDeleteRenderbuffers(1, &depthStencil);
  glDeleteFramebuffers(1, &fbo);
}

/**
* �t���[���o�b�t�@��`��Ώۂɐݒ�.
*/
void FramebufferObject::Bind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glViewport(0, 0, width, height);
}

/**
* �`��Ώۂ��f�t�H���g�t���[���o�b�t�@�ɖ߂�.
*/
void FramebufferObject::Unbind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  int w, h;
  GLFWwindow* window = GameData::Get().window;
  glfwGetWindowSize(window, &w, &h);
  glViewport(0, 0, w, h);
}

/**
* �J���[�e�N�X�`�����O���t�B�b�N�X�p�C�v���C���Ɋ��蓖�Ă�.
*
* @param unit ���蓖�Đ�̃e�N�X�`�����j�b�g�ԍ�.
*/
void FramebufferObject::BindColorTexture(GLuint unit) const
{
  texColor->Bind(unit);
}

/**
* �J���[�e�N�X�`�����O���t�B�b�N�X�p�C�v���C��������O��.
*/
void FramebufferObject::UnbindColorTexture() const
{
  texColor->Unbind();
}

/**
* �[�x�X�e���V���e�N�X�`�����O���t�B�b�N�X�p�C�v���C���Ɋ��蓖�Ă�.
*
* @param unit ���蓖�Đ�̃e�N�X�`�����j�b�g�ԍ�.
*/
void FramebufferObject::BindDepthStencilTexture(GLuint unit) const
{
  texDepthStencil->Bind(unit);
}

/**
* �[�x�X�e���V���e�N�X�`�����O���t�B�b�N�X�p�C�v���C��������O��.
*/
void FramebufferObject::UnbindDepthStencilTexture() const
{
  texDepthStencil->Unbind();
}

