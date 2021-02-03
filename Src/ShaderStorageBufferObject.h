/**
* @file ShaderStorageBufferObject.h
*/
#ifndef SHADERSTORAGEBUFFEROBJECT_H_INLCUDED
#define SHADERSTORAGEBUFFEROBJECT_H_INLCUDED
#include "glad/glad.h"

#define AVOID_AMD_DRIVER_BUG_FOR_PERSISTENT_MAP

/**
* �V�F�[�_�[�E�X�g���[�W�E�o�b�t�@�E�I�u�W�F�N�g(SSBO).
*/
class ShaderStorageBufferObject
{
public:
  explicit ShaderStorageBufferObject(size_t size);
  ~ShaderStorageBufferObject();
  ShaderStorageBufferObject(const ShaderStorageBufferObject&) = delete;
  ShaderStorageBufferObject& operator=(const ShaderStorageBufferObject&) = delete;

  void CopyData(const void* data, size_t size, size_t offset) const;
  void Bind(GLuint location) const;
  void Unbind(GLuint location) const;

private:
  GLuint id = 0;     // �I�u�W�F�N�gID
  size_t size = 0;   // �o�b�t�@�̑傫��(�P��=�o�C�g).
  void* p = nullptr; // �f�[�^�̓]������w���|�C���^.
#ifdef AVOID_AMD_DRIVER_BUG_FOR_PERSISTENT_MAP
  mutable GLsync sync = nullptr;
#endif
};

#endif // SHADERSTORAGEBUFFEROBJECT_H_INLCUDED
