/**
* @file Texture.h
*/
#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
#include <glad/glad.h>
#include <string>
#include <memory>

namespace Texture {

/**
* 画像データの用途.
*/
enum class ImageType
{
  framebuffer, // フレームバッファ
  depthbuffer, // 深度バッファ
  color,       // カラー画像
  non_color,   // 非カラー画像(メタルネスなど)
};

/**
* 2Dテクスチャ.
*/
class Image2D
{
public:
  Image2D() = default;
  explicit Image2D(const char* filename, ImageType imageType = ImageType::color);
  Image2D(const char* name, GLsizei width, GLsizei height, const void* data,
    GLenum format, GLenum type, ImageType imageType);
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

std::shared_ptr<Image2D> CreateImage2D(const char*, ImageType imageType);
void ClearTextureCache();

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
  void SetShadowMode(GLenum compareFunc);
  void Bind(GLuint unit) const;
  void Unbind() const;

private:
  GLuint id = 0;
};
void UnbindAllSamplers();

} // namespace Texture

#endif // TEXTURE_H_INCLUDED
