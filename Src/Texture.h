/**
* @file Texture.h
*/
#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
#include <glad/glad.h>
#include <string>

namespace Texture {

/**
* 2Dテクスチャ.
*/
class Image2D
{
public:
  Image2D() = default;
  explicit Image2D(const char* filename, bool isSRGB = true);
  Image2D(const char* name, GLsizei width, GLsizei height, const void* data,
    GLenum format, GLenum type, GLenum internalFormat = GL_SRGB8_ALPHA8);
  ~Image2D();
  Image2D(const Image2D&) = delete;
  Image2D& operator=(const Image2D&) = delete;

  explicit operator bool() const { return id; }

  void Bind(GLuint unit) const;
  void Unbind() const;
  GLsizei Width() const;
  GLsizei Height() const;

  // テクスチャIDを取得する.
  GLuint GetId() const { return id; }

private:
  std::string name;
  GLuint id = 0;
  GLsizei width = 0;
  GLsizei height = 0;
};
void UnbindTexture(GLuint unit);
void UnbindAllTextures();

/**
* サンプラ.
*/
class Sampler
{
public:
  Sampler();
  ~Sampler();
  Sampler(const Sampler&) = delete;
  Sampler& operator=(const Sampler&) = delete;

  explicit operator bool() const { return id; }

  void SetWrapMode(GLenum mode);
  void SetFilter(GLenum filter);
  void Bind(GLuint unit) const;
  void Unbind() const;

private:
  GLuint id = 0;
};
void UnbindAllSamplers();

} // namespace Texture

#endif // TEXTURE_H_INCLUDED
