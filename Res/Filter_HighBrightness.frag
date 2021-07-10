#version 450

// 入力変数
layout(location=1) in vec2 inTexCoord;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;

const float threshold = 1.0; // 明るいと判定するしきい値.

// フラグメントシェーダプログラム
void main()
{
#if 1 // ボックスフィルタの有無
  vec4 unitSize;
  unitSize.xy = vec2(0.75) / vec2(textureSize(texColor, 0));
  unitSize.zw = -unitSize.xy;

  fragColor.rgb  = texture(texColor, inTexCoord + unitSize.xy).rgb;
  fragColor.rgb += texture(texColor, inTexCoord + unitSize.zy).rgb;
  fragColor.rgb += texture(texColor, inTexCoord + unitSize.xw).rgb;
  fragColor.rgb += texture(texColor, inTexCoord + unitSize.zw).rgb;
  fragColor.rgb *= 0.25;
#else
  fragColor.rgb  = texture(texColor, inTexCoord).rgb;
#endif

  float brightness = max(fragColor.r, max(fragColor.g, fragColor.b));
  float contribution = max(brightness - threshold, 0.0) / max(brightness, 0.00001);
  fragColor.rgb *= contribution;
  fragColor.a = 1.0;
}
