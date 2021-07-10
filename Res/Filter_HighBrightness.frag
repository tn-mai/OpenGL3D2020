#version 450

// ���͕ϐ�
layout(location=1) in vec2 inTexCoord;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor;

const float threshold = 1.0; // ���邢�Ɣ��肷�邵�����l.

// �t���O�����g�V�F�[�_�v���O����
void main()
{
#if 1 // �{�b�N�X�t�B���^�̗L��
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
