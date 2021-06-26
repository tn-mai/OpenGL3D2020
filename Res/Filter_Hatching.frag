#version 450

#define USE_REAL_Z 1

// ���͕ϐ�
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor;
layout(binding=1) uniform sampler2D texHatching;
layout(binding=2) uniform sampler2D texDepth;

// �|�A�\���f�B�X�N�z��
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

// �J�����p�����[�^(�P��:���[�g��).
const float cameraNear = 1.0;   // �J�����̃j�A���ʂ܂ł̋���.
const float cameraFar = 500.0;  // �J�����̃t�@�[���ʂ܂ł̋���.

// �{�P�p�����[�^(�P��:���[�g��).
const float focusPoint = 10.0; // �J�������璍���_�܂ł̋���
const float bokehStart = 2.0;  // �{�P�n�߂�͈�
const float bokehEnd   = 6.0;  // �{�P���ő�ɂȂ�͈�
const float bokehPower = 5.0;  // �{�P�̋���

/**
* ���s������{�P�x�������v�Z.
*/
float GetBokehValue()
{
  // glm::perspective�֐��̋t�̌v�Z���s���āA�[�x�o�b�t�@�̒l�����_����̃��[�g�������ɕϊ�.
  float z = texture(texDepth, inTexcoord).r;
  z = 2.0 * z - 1.0;
  z = -(2 * cameraNear * cameraFar) / (cameraFar + cameraNear - z * (cameraFar - cameraNear));

  // �{�P�x�������v�Z.
  z = abs(z + focusPoint);
  z = max(z - bokehStart, 0.0);
  z = min(z / (bokehEnd - bokehStart), 1.0);
  return z;
}

/**
* �{�P�x�����ɉ����ĉ摜���ڂ���.
*
* @param texBokeh  �ڂ����Ɏg���e�N�X�`��.
* @param bokeh     �{�P�x����(0.0�`1.0).
*
* @return �ڂ������s�N�Z���̐F.
*/
vec4 ApplyBokehEffect(sampler2D texBokeh, float bokeh)
{
  // �~�`�͈̔͂̃s�N�Z����ǂݍ���. z�ɉ����ĉ~�̔��a���ς��.
  vec2 subPixelSize = vec2(1.0) / textureSize(texBokeh, 0);
  vec4 bokehColor = texture(texBokeh, inTexcoord);
  for (int i = 0; i < poissonSampleCount; ++i) {
    vec2 offset = subPixelSize * poissonDisc[i] * bokeh * bokehPower;
    bokehColor += texture(texBokeh, inTexcoord + offset);
  }

  // ���ς����߂�.
  bokehColor *= 1.0 / float(poissonSampleCount + 1);

  return bokehColor;
}

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
  float bokeh = GetBokehValue();
  fragColor = inColor * ApplyBokehEffect(texColor, bokeh);

  vec2 screenSize = vec2(1280, 720);
  vec2 texPencilSize = textureSize(texHatching, 0);
  vec2 texcoord = fract(inTexcoord * (screenSize / texPencilSize));
  vec3 pencil = texture(texHatching, texcoord).rgb;

  // �ł����邢�v�f��S�̖̂��邳�Ƃ���.
  // YUV�Ȃǂ̋P�x�v�Z�ł͐��قڎ���ł��܂����߁A�e�̔���ɂ͌����Ă��Ȃ�.
  float brightness = max(fragColor.r, max(fragColor.g, fragColor.b));

  // ���邳0.3�������u�Â��̈�v�Ƃ��āA0.1�܂łȂ߂炩�ɍ����䗦��ω�������.
  float ratio = smoothstep(0.1, 0.3, brightness); // �����䗦.

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
  fragColor.rgb *= mix(pencil * outline, vec3(1.0), bokeh);
}
