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
* �摜�f�[�^�̗p�r.
*/
enum class ImageType
{
  framebuffer, // �t���[���o�b�t�@
  depthbuffer, // �[�x�o�b�t�@
  color,       // �J���[�摜
  non_color,   // ��J���[�摜(���^���l�X�Ȃ�)
};

/**
* 2D�e�N�X�`��.
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

  // �e�N�X�`��ID���擾����.
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
* �T���v��.
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
