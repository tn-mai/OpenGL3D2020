/**
* @file ShaderStorageBufferObject.cpp
*/
#include "ShaderStorageBufferObject.h"
#include "GLContext.h"
#include <iostream>

/**
* �R���X�g���N�^.
*
* @param size  SSBO�̑傫��(�o�C�g��).
*/
ShaderStorageBufferObject::ShaderStorageBufferObject(size_t size)
{
  id = GLContext::CreateBuffer(size, nullptr,
    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
  p = glMapNamedBufferRange(id, 0, size,
    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
  this->size = size;
}

/**
* �f�X�g���N�^.
*/
ShaderStorageBufferObject::~ShaderStorageBufferObject()
{
  glUnmapNamedBuffer(id);
  glDeleteBuffers(1, &id);
}

/**
* �f�[�^��SSBO�ɃR�s�[����.
*
* @param data �R�s�[����f�[�^�̃A�h���X.
* @param size �R�s�[����o�C�g��.
* @param offset �R�s�[��̐擪�ʒu(�o�C�g�P��).
*/
void ShaderStorageBufferObject::CopyData(
  const void* data, size_t size, size_t offset) const
{
  if (offset + size > this->size) {
    std::cerr << "[�G���[]" << __func__ <<
      ": �T�C�Y�܂��̓I�t�Z�b�g���傫�����܂�(size=" << size <<
      ",offset=" << offset << "/" << this->size << ").\n";
    return;
  }
  memcpy(static_cast<char*>(p) + offset, data, size);
}

/**
* SSBO���O���t�B�b�N�X�p�C�v���C���Ɋ��蓖�Ă�.
*
* @param location ���蓖�Đ�̃��P�[�V�����ԍ�.
*/
void ShaderStorageBufferObject::Bind(GLuint location) const
{
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location, id);
}

/**
* SSBO�̃O���t�B�b�N�X�p�C�v���C���ւ̊��蓖�Ă���������.
*
* @param location ���蓖�Đ�̃��P�[�V�����ԍ�.
*/
void ShaderStorageBufferObject::Unbind(GLuint location) const
{
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location, 0);
}

