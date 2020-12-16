#version 450

// ���͕ϐ�
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor;

// �K����.
const float levels = 5;

// �t���O�����g�V�F�[�_�v���O����
void main()
{
  // �J���[���擾.
  fragColor = inColor * texture(texColor, inTexcoord);

  // �ł����邢�v�f��S�̖̂��邳�Ƃ���.
  // YUV�Ȃǂ̋P�x�v�Z�ł͐��قڎ���ł��܂����߃|�X�^�[���Ɍ����Ă��Ȃ�.
  float brightness = max(fragColor.r, max(fragColor.g, fragColor.b));

  // level�i�K�ŗʎq�����ꂽ���邳���v�Z.
  float quantizedBrightness = floor(brightness * levels + 0.5) / levels;

  // �ʎq�����ꂽ���邳�Ǝ��ۂ̖��邳�̔���J���[�Ɋ|���邱�ƂŁA���ۂ̃J���[��ʎq������.
  fragColor.rgb *= quantizedBrightness / brightness;
}

