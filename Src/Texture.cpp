/**
* @file Texture.cpp
*/
#include "Texture.h"
#include "GLContext.h"
#include <unordered_map>
#include <iostream>

// インテルグラフィックスドライバのバグによりglBindTextureUnit(i, 0)が機能しないことへの対応.
#define AVOID_INTEL_DRIVER_BUG_FOR_GLBINDTEXTUREUNIT

// AMDドライバのバグによりglBindTextures, glBindTextureUnitを使うとバインド解除が遅延する問題への対応.
// glBindTextures, glBindTextureUnitによってテクスチャをバインドしたとき、
// バインド解除にglBindTextures, glBindTextureUnitを使うと直後の描画ではバインドが解除されないことがある.
// 同じテクスチャを「バインド->アンバインド」する場合は問題ないが、「バインド*n->アンバインド」とすると遅延が発生する.
// 参照カウントを疑ったが「バインド*n->アンバインド*n」でも遅延するので、そうでない可能性がある.
// しかし、アンバインドにglActiveTexture&glBindTextureを使うと遅延せず、直後の描画コマンドでも正しくアンバインドされている.
// また、バインドにglActiveTexture&glBindTextureを使うと、どの方法でも1回で正しくアンバインドされる.
// バインドに`glActiveTexture&glBindTexture`を使っておけばアンバインドはどの方法でも機能するように見えるが、
// 正直なところ、バインドとアンバインドで異なる関数を使った結果、またバグを踏むかもしれず不安は残る.
// 結局、"AMDドライバではglBindTextures, glBindTextureUnitを使うな"ということになる.
// なおAMDは既にOpenGLドライバの更新を終了してVulkanに移行している. この問題が改善される日は来ないだろう.
#define AVOID_AMD_DRIVER_BUG_FOR_GLBINDTEXTURES

/**
* テクスチャ関連の機能を格納する名前空間.
*/
namespace Texture {

namespace /* unnamed */ {

/**
* テクスチャのバインド状態を追跡するための配列.
*
* テクスチャイメージユニットにバインドされたテクスチャIDを保持する.
*/
GLuint textureBindingState[16] = {};

/**
* サンプラのバインド状態を追跡するための配列.
*/
GLuint samplerBindingState[16] = {};

/**
* テクスチャ・キャッシュ.
*/
std::unordered_map<std::string, std::shared_ptr<Image2D>> textureCache;

} // unnamed namespace

/**
* テクスチャのバインドを解除する.
*
* @param unit 解除するバインディングポイント.
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
* 全てのテクスチャのバインドを解除する.
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
* 全てのサンプラのバインドを解除する.
*/
void UnbindAllSamplers()
{
  for (GLuint i = 0; i < std::size(samplerBindingState); ++i) {
    samplerBindingState[i] = 0;
  }
  glBindSamplers(0, static_cast<GLsizei>(std::size(samplerBindingState)), samplerBindingState);
}

/**
* 2Dテクスチャを作成する.
*
* @param filename 2Dテクスチャとして読み込むファイル名.
* @param imageType 画像の種類.
* 
* @return 作成したテクスチャ.
*/
std::shared_ptr<Image2D> CreateImage2D(const char* filename, ImageType imageType)
{
  // キャッシュを検索し、同名のテクスチャが見つけたら、見つけたテクスチャを返す.
  auto itr = textureCache.find(filename);
  if (itr != textureCache.end()) {
    return itr->second;
  }

  // キャッシュに同名のテクスチャがなかったので、新しくテクスチャを作る.
  // 作成したテクスチャをテクスチャキャッシュに追加し、さらに戻り値として返す.
  std::shared_ptr<Image2D> p = std::make_shared<Image2D>(filename, imageType);
  textureCache.emplace(filename, p);
  return p;
}

/**
* テクスチャキャッシュを空にする.
*/
void ClearTextureCache()
{
  textureCache.clear();
}

/**
* 画像の種類に対応するGPU側のピクセル形式を取得する.
*
* @param imageType 画像の種類.
*
* @return typeに対応するGPU側のピクセル形式.
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
* コンストラクタ.
*
* @param filename 2Dテクスチャとして読み込むファイル名.
* @param imageType 画像の種類.
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
    std::cout << "[情報]" << __func__ << "テクスチャ" << filename << "を作成(" <<
      width << "x" << height << " " << ifname << ").\n";
  }
}

/**
* コンストラクタ.
*
* @param name    テクスチャを識別するための名前.
* @param width   画像の幅(ピクセル数).
* @param height  画像の高さ(ピクセル数).
* @param data    画像データへのポインタ.
* @param pixelFormat  画像のピクセル形式(GL_BGRAなど).
* @param type    画像データの型.
* @param imageType 画像の種類.
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
    std::cout << "[情報]" << __func__ << "テクスチャ" << name << "を作成(" <<
      width << "x" << height << " " << pfname << " " << ifname << ").\n";
  }
}

/**
* デストラクタ.
*/
Image2D::~Image2D()
{
  Unbind();
  glDeleteTextures(1, &id);
}

/**
* テクスチャをテクスチャイメージユニットにバインドする.
*
* @param unit バインド先のユニット番号.
*/
void Image2D::Bind(GLuint unit) const
{
  if (unit >= std::size(textureBindingState)) {
    std::cerr << "[エラー]" << __func__ << ": ユニット番号が大きすぎます(unit=" << unit << ")\n";
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
* テクスチャのバインドを解除する.
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
* テクスチャの幅を取得する.
*
* @return テクスチャの幅(ピクセル数).
*/
GLsizei Image2D::Width() const
{
  return width;
}

/**
* テクスチャの高さを取得する.
*
* @return テクスチャの高さ(ピクセル数).
*/
GLsizei Image2D::Height() const
{
  return height;
}

/**
* コンストラクタ.
*/
Sampler::Sampler() : id(GLContext::CreateSampler())
{
}

/**
* デストラクタ.
*/
Sampler::~Sampler()
{
  Unbind();
  glDeleteSamplers(1, &id);
}

/**
* ラップモードを指定する.
*
* @param mode ラップモード.
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
* フィルタを設定する.
*
* @param filter 設定するフィルタの種類(GL_LINEARかGL_NEAREST).
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
* デプスシャドウマップ用の深度比較モードを設定する.
*
* @param compareFunc 設定する深度比較モード(GL_LEQUALなど).
*                    深度比較モードを無効化するにはGL_NONEを指定する.
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
* サンプラをテクスチャイメージユニットにバインドする.
*
* @param unit バインド先のユニット番号.
*/
void Sampler::Bind(GLuint unit) const
{
  if (unit >= std::size(textureBindingState)) {
    std::cerr << "[エラー]" << __func__ << ": ユニット番号が大きすぎます(unit=" << unit << ")\n";
    return;
  }
  glBindSampler(unit, id);
  samplerBindingState[unit] = id;
}

/**
* サンプラのバインドを解除する.
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

