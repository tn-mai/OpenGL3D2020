#version 450

#define USE_REAL_Z 1

// 入力変数
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;
layout(binding=1) uniform sampler2D texHatching;
layout(binding=2) uniform sampler2D texDepth;

// ポアソンディスク配列
// https://github.com/spite/Wagner/blob/master/fragment-shaders/poisson-disc-blur-fs.glsl
const int poissonSampleCount = 12;
const vec2 poissonDisc[poissonSampleCount] = {
  {-0.326,-0.406},
  {-0.840,-0.074},
  {-0.696, 0.457},
  {-0.203, 0.621},
  { 0.962,-0.195},
  { 0.473,-0.480},
  { 0.519, 0.767},
  { 0.185,-0.893},
  { 0.507, 0.064},
  { 0.896, 0.412},
  {-0.322,-0.933},
  {-0.792,-0.598}
};

// カメラパラメータ(単位:メートル).
const float cameraNear = 1.0;   // カメラのニア平面までの距離.
const float cameraFar = 500.0;  // カメラのファー平面までの距離.

// ボケパラメータ(単位:メートル).
const float focusPoint = 10.0; // カメラから注視点までの距離
const float bokehStart = 2.0;  // ボケ始める範囲
const float bokehEnd   = 6.0;  // ボケが最大になる範囲
const float bokehPower = 5.0;  // ボケの強さ

/**
* 奥行きからボケ度合いを計算.
*/
float GetBokehValue()
{
  // glm::perspective関数の逆の計算を行って、深度バッファの値を視点からのメートル距離に変換.
  float z = texture(texDepth, inTexcoord).r;
  z = 2.0 * z - 1.0;
  z = -(2 * cameraNear * cameraFar) / (cameraFar + cameraNear - z * (cameraFar - cameraNear));

  // ボケ度合いを計算.
  z = abs(z + focusPoint);
  z = max(z - bokehStart, 0.0);
  z = min(z / (bokehEnd - bokehStart), 1.0);
  return z;
}

/**
* ボケ度合いに応じて画像をぼかす.
*
* @param texBokeh  ぼかしに使うテクスチャ.
* @param bokeh     ボケ度合い(0.0～1.0).
*
* @return ぼかしたピクセルの色.
*/
vec4 ApplyBokehEffect(sampler2D texBokeh, float bokeh)
{
  // 円形の範囲のピクセルを読み込む. zに応じて円の半径が変わる.
  vec2 subPixelSize = vec2(1.0) / textureSize(texBokeh, 0);
  vec4 bokehColor = texture(texBokeh, inTexcoord);
  for (int i = 0; i < poissonSampleCount; ++i) {
    vec2 offset = subPixelSize * poissonDisc[i] * bokeh * bokehPower;
    bokehColor += texture(texBokeh, inTexcoord + offset);
  }

  // 平均を求める.
  bokehColor *= 1.0 / float(poissonSampleCount + 1);

  return bokehColor;
}

float GetZ(vec2 offset)
{
  float w = texture(texDepth, inTexcoord + offset).r;
#if USE_REAL_Z
  float near = 1;
  float far = 500;
  float n = 2 * w - 1; // ±1の範囲に変換.
  n = -2 * near * far / (far + near - n * (far - near));
  return -n / (far - near);
#else
  return w;
#endif
}

// フラグメントシェーダプログラム
void main()
{
  // カラーを取得.
  float bokeh = GetBokehValue();
  fragColor = inColor * ApplyBokehEffect(texColor, bokeh);

  vec2 screenSize = vec2(1280, 720);
  vec2 texPencilSize = textureSize(texHatching, 0);
  vec2 texcoord = fract(inTexcoord * (screenSize / texPencilSize));
  vec3 pencil = texture(texHatching, texcoord).rgb;

  // 最も明るい要素を全体の明るさとする.
  // YUVなどの輝度計算では青がほぼ死んでしまうため、影の判定には向いていない.
  float brightness = max(fragColor.r, max(fragColor.g, fragColor.b));

  // 明るさ0.3未満を「暗い領域」として、0.1までなめらかに合成比率を変化させる.
  float ratio = smoothstep(0.1, 0.3, brightness); // 合成比率.

  // 暗い領域は斜線テクスチャの色、明るい領域は白色になるように合成.
  pencil = mix(pencil, vec3(1), ratio);

  // 周囲8個のピクセルとの差分を計算(ラプラシアンフィルタ).
  vec2 pixelSize = vec2(1) / textureSize(texDepth, 0);
  float outline = 8 * texture(texDepth, inTexcoord).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2(-1, 1)).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2( 0, 1)).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2( 1, 1)).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2(-1, 0)).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2( 1, 0)).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2(-1,-1)).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2( 0,-1)).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2( 1,-1)).r;
  outline *= texture(texDepth, inTexcoord).r; // 奥行きによる変化量の差をなくす.
  outline = 1 - smoothstep(0.0, 0.01, abs(outline)); // 輪郭線ではない部分を1にする.

  // 斜線と輪郭の色を元の色に乗算.
  fragColor.rgb *= mix(pencil * outline, vec3(1.0), bokeh);
}
