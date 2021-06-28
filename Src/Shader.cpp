/**
* @file Shader.cpp
*/
#include "Shader.h"
#include "GLContext.h"
#include <iostream>

// for Test
#include <vector>
#include <fstream>

// Radeonでパイプラインオブジェクトを使うとGL_COMPARE_REF_TO_TEXTUREが有効にならない問題への対処.
// プログラムオブジェクトを使うと問題なく機能する.
//#define AVOID_TEXTURE_COMPARE_MODE_ISSUE

/**
* シェーダに関する機能を格納する名前空間.
*/
namespace Shader {

namespace Test {

/**
* シェーダー・プログラムをコンパイルする
*
* @param type シェーダーの種類
* @param string シェーダー・プログラムへのポインタ
*
* @retval 0 より大きい　作成したシェーダー・オブジェクト
* @retval 0				シェーダー・オブジェクトの作成に失敗
*/
GLuint Compile(GLenum type, const GLchar* string)
{
  if (!string) {
    return 0;
  }

  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &string, nullptr);
  glCompileShader(shader);
  GLint compiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  // コンパイルに失敗した場合、原因をコンソールに出力して０を返す
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen) {
      std::vector<char> buf;
      buf.resize(infoLen);
      if (static_cast<int>(buf.size()) >= infoLen) {
        glGetShaderInfoLog(shader, infoLen, NULL, buf.data());
        std::cerr << "ERROR: シェーダーのコンパイルに失敗.\n" << buf.data() << std::endl;
      }
    }
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

/**
* プログラム・オブジェクトを作成する
*
* @param vsCode 頂点シェーダー・プログラムへのポインタ
* @param fsCode フラグメントシェーダー・プログラムへのポインタ
*
* @retval 0より大きい 作成したプログラム・オブジェクト
* @retval 0				プログラム・オブジェクトの作成に失敗
*/
GLuint Build(const GLchar* vsCode, const GLchar* fsCode)
{
  GLuint vs = Compile(GL_VERTEX_SHADER, vsCode);
  GLuint fs = Compile(GL_FRAGMENT_SHADER, fsCode);
  if (!vs || !fs) {
    return 0;
  }
  GLuint program = glCreateProgram();
  glAttachShader(program, fs);
  glDeleteShader(fs);
  glAttachShader(program, vs);
  glDeleteShader(vs);
  glLinkProgram(program);
  GLint linkStatus = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
  if (linkStatus != GL_TRUE) {
    GLint infoLen = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen) {
      std::vector<char> buf;
      buf.resize(infoLen);
      if (static_cast<int>(buf.size()) >= infoLen) {
        glGetProgramInfoLog(program, infoLen, NULL, buf.data());
        std::cerr << "ERROR: シェーダーリンクに失敗.\n" << buf.data() << std::endl;
      }
    }
    glDeleteProgram(program);
    return 0;
  }
  return program;
}

/**
* ファイルを読み込む
*
* @param path 読み込むファイル名
*
* @return 読み込んだデータ
*/
std::vector<GLchar> ReadFile(const char* path)
{
  std::basic_ifstream<GLchar> ifs;
  ifs.open(path, std::ios_base::binary);
  if (!ifs.is_open()) {
    std::cerr << "ERROR: " << path << " を開けません.\n";
    return{};
  }
  ifs.seekg(0, std::ios_base::end);
  const size_t length = (size_t)ifs.tellg();
  ifs.seekg(0, std::ios_base::beg);
  std::vector<GLchar> buf(length);
  ifs.read(buf.data(), length);
  buf.push_back('\0');
  return buf;
}
/**
* ファイルからプログラム・オブジェクトを作成する
*
* @param vsPath 頂点シェーダー・ファイル名
* @param fsPath フラグメントシェーダー・ファイル名
*
* @return 作成したプログラム・オブジェクト
*/
GLuint BuildFromFile(const char* vsPath, const char* fsPath)
{
  const std::vector<GLchar> vsCode = ReadFile(vsPath);
  const std::vector<GLchar> fsCode = ReadFile(fsPath);
  return Build(vsCode.data(), fsCode.data());
}

} // namespace Test

/**
* コンストラクタ.
*
* @param vsCode  頂点シェーダー・プログラムのアドレス.
* @param fsCode  フラグメントシェーダー・プログラムのアドレス.
*/
Pipeline::Pipeline(const char* vsFilename, const char* fsFilename)
{
#ifndef AVOID_TEXTURE_COMPARE_MODE_ISSUE
  vp = GLContext::CreateProgramFromFile(GL_VERTEX_SHADER, vsFilename);
  fp = GLContext::CreateProgramFromFile(GL_FRAGMENT_SHADER, fsFilename);
  id = GLContext::CreatePipeline(vp, fp);

  // 光源データ転送先のプログラムIDを設定.
  if (glGetUniformLocation(vp, "directionalLight.direction") >= 0) {
    lightingProgram = vp;
  } else if (glGetUniformLocation(fp, "directionalLight.direction") >= 0) {
    lightingProgram = fp;
  }
#else
  id = vp = fp = Test::BuildFromFile(vsFilename, fsFilename);
  lightingProgram = vp;
#endif
  // オブジェクトカラーの初期値を設定.
  if (glGetUniformLocation(vp, "objectColor") >= 0) {
    SetObjectColor(glm::vec4(1));
  }
}

/**
* デストラクタ.
*/
Pipeline::~Pipeline()
{
#ifndef AVOID_TEXTURE_COMPARE_MODE_ISSUE
  glDeleteProgramPipelines(1, &id);
  glDeleteProgram(fp);
#endif
  glDeleteProgram(vp);
}

/**
* プログラムパイプラインをバインドする.
*/
void Pipeline::Bind() const
{
#ifndef AVOID_TEXTURE_COMPARE_MODE_ISSUE
  glUseProgram(0);
  glBindProgramPipeline(id);
#else
  glUseProgram(vp);
#endif
}

/**
* プログラムパイプラインのバインドを解除する.
*/
void Pipeline::Unbind() const
{
#ifndef AVOID_TEXTURE_COMPARE_MODE_ISSUE
  glBindProgramPipeline(0);
#else
  glUseProgram(0);
#endif
}

/**
* シェーダにMVP行列を設定する.
*
* @param matMVP 設定するMVP行列.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetMVP(const glm::mat4& matMVP) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locMatMVP = 0;
  glProgramUniformMatrix4fv(vp, locMatMVP, 1, GL_FALSE, &matMVP[0][0]);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":MVP行列の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* シェーダにモデル行列を設定する.
*
* @param matModel 設定する法線行列.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetModelMatrix(const glm::mat4& matModel) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locMatModel = 1;
  glProgramUniformMatrix4fv(vp, locMatModel, 1, GL_FALSE, &matModel[0][0]);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":モデル行列の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* シェーダにシャドウ行列を設定する.
*
* @param matShadow 設定するシャドウ行列.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetShadowMatrix(const glm::mat4& matShadow) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locMatShadow = 11;
  glProgramUniformMatrix4fv(vp, locMatShadow, 1, GL_FALSE, &matShadow[0][0]);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":シャドウ行列の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* シェーダにライトデータを設定する.
*
* @param light 設定するライトデータ.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetLight(const DirectionalLight& light) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locDirLight = 2;

  glProgramUniform4fv(lightingProgram, locDirLight, 1, &light.direction.x);
  glProgramUniform4fv(lightingProgram, locDirLight + 1, 1, &light.color.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":平行光源の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* シェーダにライトデータを設定する.
*
* @param light 設定するライトデータ.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetLight(const PointLight& light) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locPointLight = 4;

  glProgramUniform4fv(lightingProgram, locPointLight, 1, &light.position.x);
  glProgramUniform4fv(lightingProgram, locPointLight + 1, 1, &light.color.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":点光源の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* シェーダに環境光を設定する.
*
* @param color 設定する環境光.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetAmbientLight(const glm::vec3& color) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locAmbientLight= 6;

  glProgramUniform3fv(lightingProgram, locAmbientLight, 1, &color.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":環境光の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* シェーダにオブジェクトの色を設定する.
*
* @param color 設定する色.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetObjectColor(const glm::vec4& color) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locObjectColor = 8;

  glProgramUniform4fv(vp, locObjectColor, 1, &color.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":オブジェクトカラーの設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* モーフィングの合成比率を設定する.
*
* @param weight モーフィングの合成比率(0.0～1.0の範囲で指定すること).
*/
bool Pipeline::SetMorphWeight(const glm::vec3& weight) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locMorphRatio = 10;

  glProgramUniform3f(vp, locMorphRatio, weight.x, weight.y, weight.z);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":モーフィング合成比率の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* シェーダに視点座標を設定する.
*
* @param position 設定する視点座標.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetViewPosition(const glm::vec3& position) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locViewPosition = 7;
  glProgramUniform3f(fp, locViewPosition, position.x, position.y, position.z);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":視点座標の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* シェーダにタイマー値を設定する.
*
* @param time 設定するタイマー値.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetEffectTimer(float time) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locTime = 9;
  glProgramUniform1f(fp, locTime, time);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":タイマー値の設定に失敗.\n";
    return false;
  }
  return true;
}
/**
* プログラムパイプラインのバインドを解除する.
*/
void UnbindPipeline()
{
#ifndef AVOID_TEXTURE_COMPARE_MODE_ISSUE
  glBindProgramPipeline(0);
#else
  glUseProgram(0);
#endif
}

} // namespace Shader

