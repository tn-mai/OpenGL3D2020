#version 450

#define USE_REAL_Z 0

// ���͕ϐ�
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor;
layout(binding=1) uniform sampler2D texHatching;
layout(binding=2) uniform sampler2D texDepth;

// �������l.
const float threshold = 0.25;

float GetZ(vec2 offset)
{
  float w = texture(texDepth, inTexcoord + offset).r;
#if USE_REAL_Z
  float near = 1;
  float far = 500;
  float n = 2 * w - 1; // �}1�͈̔͂ɕϊ�.
  n = -2 * near * far / (far + near - n * (far - near));
  return -n / (far - near);
#else
  return w;
#endif
}

// �t���O�����g�V�F�[�_�v���O����
void main()
{
  // �J���[���擾.
  fragColor = inColor * texture(texColor, inTexcoord);
  vec2 screenSize = vec2(1280, 720);
  vec2 texPencilSize = textureSize(texHatching, 0);
  vec2 texcoord = fract(inTexcoord * (screenSize / texPencilSize));
  vec3 pencil = texture(texHatching, texcoord).rgb;

  // �ł����邢�v�f��S�̖̂��邳�Ƃ���.
  // YUV�Ȃǂ̋P�x�v�Z�ł͐��قڎ���ł��܂����߁A�e�̔���ɂ͌����Ă��Ȃ�.
  float brightness = max(fragColor.r, max(fragColor.g, fragColor.b));

  // ���邳0.5�������u�Â��̈�v�Ƃ��āA0.3�܂łȂ߂炩�ɍ����䗦��ω�������.
  float ratio = smoothstep(0.2, 0.4, brightness); // �����䗦.

  // �Â��̈�͎ΐ��e�N�X�`���̐F�A���邢�̈�͔��F�ɂȂ�悤�ɍ���.
  pencil = mix(pencil, vec3(1), ratio);

  // ����8�̃s�N�Z���Ƃ̍������v�Z(���v���V�A���t�B���^).
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
  outline *= texture(texDepth, inTexcoord).r; // ���s���ɂ��ω��ʂ̍����Ȃ���.
  outline = 1 - smoothstep(0.0, 0.01, abs(outline)); // �֊s���ł͂Ȃ�������1�ɂ���.

  // �ΐ��Ɨ֊s�̐F�����̐F�ɏ�Z.
  fragColor.rgb *= pencil * outline;
}
