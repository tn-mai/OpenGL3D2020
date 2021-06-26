/**
* @file GLContext.h
**/
#ifndef GLCONTEXT_H_INCLUDED
#define GLCONTEXT_H_INCLUDED
#include <glad/glad.h>
#include <string>
#include <memory>

/// �O�������W�^.
struct Position
{
  float x, y, z;
};

/// RGBA�J���[�^.
struct Color
{
  float r, g, b, a;
};

namespace GLContext {

GLuint CreateBuffer(GLsizeiptr size, const GLvoid* data, GLbitfield flags = 0);
GLuint CreateVertexArray(GLuint vboPosition, GLuint vboColor,
  GLuint vboTexcoord, GLuint vboNormal, GLuint ibo);
void SetMorphBaseMesh(GLuint vao, GLuint vboPosition, GLuint vboColor,
  GLuint vboTexcoord, GLuint vboNormal, GLuint baseVertex);
void SetMorphTargetMesh(GLuint vao, GLuint vboPosition,
  GLuint vboNormal, GLuint baseVertex);
void SetPreviousMorphMesh(GLuint vao, GLuint vboPosition,
  GLuint vboNormal, GLuint baseVertex0, GLuint baseVertex1);
GLuint CreateProgram(GLenum type, const GLchar* code);
GLuint CreateProgramFromFile(GLenum type, const char* filename);
GLuint CreatePipeline(GLuint vp, GLuint fp);
GLuint CreateImage2D(GLsizei width, GLsizei height, const void* data, GLenum format, GLenum type, GLenum internalFormat);
GLuint CreateImage2D(const char* filename, GLenum internalFormat);
GLuint CreateSampler();

} // namespace GLContext

#endif // GLCONTEXT_H_INCLUDED

