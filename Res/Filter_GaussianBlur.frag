#version 450

// ���͕ϐ�
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor;

// �ڂ����T�C�Y.
const int size = 5;

// �d�ݔz��.
const float kernel[size][size] = { 
  { 0.003765, 0.015019, 0.023792, 0.015019, 0.003765 },
  { 0.015019, 0.059912, 0.094907, 0.059912, 0.015019 },
  { 0.023792, 0.094907, 0.150342, 0.094907, 0.023792 },
  { 0.015019, 0.059912, 0.094907, 0.059912, 0.015019 },
  { 0.003765, 0.015019, 0.023792, 0.015019, 0.003765 },
};

// �t���O�����g�V�F�[�_�v���O����
void main()
{
  vec2 unitSize = vec2(1) / vec2(textureSize(texColor, 0));

  vec2 texcoord;
  vec3 c = vec3(0);
  texcoord.y = inTexcoord.y - unitSize.y * float(size);
  for (int y = 0; y < size; ++y) {
    texcoord.x = inTexcoord.x - unitSize.x * float(size);
    for (int x = 0; x < size; ++x) {
      c += texture(texColor, texcoord).rgb * kernel[y][x];
      texcoord.x += unitSize.x;
    }
    texcoord.y += unitSize.y;
  }

  fragColor = inColor * vec4(c, 1);
}

