/**
* @file Mesh.h
**/
#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <vector>

namespace Mesh {

// ��s�錾.
class PrimitiveBuffer;

/**
* �v���~�e�B�u�f�[�^.
*/
class Primitive
{
public:
  Primitive() = default;
  Primitive(GLenum m, GLsizei c, GLsizeiptr o, GLint b, const PrimitiveBuffer* pb) :
    mode(m), count(c), indices(reinterpret_cast<GLvoid*>(o)), baseVertex(b), primitiveBuffer(pb)
  {}
  ~Primitive() = default;

  void Draw(const Primitive* morphTarget = nullptr) const;

private:
  GLenum mode = GL_TRIANGLES; ///< �v���~�e�B�u�̎��.
  GLsizei count = 0; ///< �`�悷��C���f�b�N�X��.
  const GLvoid* indices = 0; ///< �`��J�n�C���f�b�N�X�̃o�C�g�I�t�Z�b�g.
  GLint baseVertex = 0; ///< �C���f�b�N�X0�ԂƂ݂Ȃ���钸�_�z����̈ʒu.
  const PrimitiveBuffer* primitiveBuffer = nullptr;
};

/**
* �v���~�e�B�u�o�b�t�@.
*/
class PrimitiveBuffer
{
public:
  PrimitiveBuffer() = default;
  ~PrimitiveBuffer();
  PrimitiveBuffer(const PrimitiveBuffer&) = delete;
  PrimitiveBuffer& operator=(const PrimitiveBuffer&) = delete;

  // �������Ǘ�.
  bool Allocate(GLsizei maxVertexCount, GLsizei maxIndexCount);
  void Free();

  // �v���~�e�B�u�̒ǉ��ƎQ��.
  bool Add(size_t vertexCount, const glm::vec3* pPosition, const glm::vec4* pColor,
    const glm::vec2* pTexcoord, const glm::vec3* pNormal, size_t indexCount, const GLushort* pIndex);
  bool AddFromObjFile(const char* filename);
  const Primitive& Get(size_t n) const;

  // VAO�o�C���h�Ǘ�.
  void BindVertexArray() const;
  void UnbindVertexArray() const;

  void SetMorphBaseMesh(GLuint offset) const;
  void SetMorphTargetMesh(GLuint offset) const;

private:
  std::vector<Primitive> primitives;

  GLuint vboPosition = 0;
  GLuint vboColor = 0;
  GLuint vboTexcoord = 0;
  GLuint vboNormal = 0;
  GLuint ibo = 0;
  GLuint vao = 0;

  GLsizei maxVertexCount = 0; // �i�[�ł���ő咸�_��.
  GLsizei curVertexCount = 0; // �i�[�ςݒ��_��.
  GLsizei maxIndexCount = 0; // �i�[�ł���ő�C���f�b�N�X��.
  GLsizei curIndexCount = 0; // �i�[�ς݃C���f�b�N�X��.
};

} // namespace Mesh


#endif // MESH_H_INCLUDED
