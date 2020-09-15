#version 450 core

// ���͕ϐ�
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor;

// �t���O�����g�V�F�[�_�v���O����
void main()
{
  if (texture(texColor, inTexcoord).a < 0.5) {
    discard;
  }
}