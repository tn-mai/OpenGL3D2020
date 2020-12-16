#version 450

// ���͕ϐ�
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor;
layout(binding=1) uniform sampler2D texDepth;

float GetZ(vec2 offset)
{
  float w = texture(texDepth, inTexcoord + offset).r;
  //return w;
  float near = 0.1;
  float far = 500;
  float n = w;//2 * w - 1; // �}1�͈̔͂ɕϊ�.
  n = -2 * near * far / (far + near - w * (far - near));
  return n / (far - near);
}

// �t���O�����g�V�F�[�_�v���O����
void main()
{
  // 8�ߖT���v���V�A���t�B���^
  vec2 unitSize = vec2(1) / vec2(textureSize(texDepth, 0));
  float c0 = GetZ(vec2(0));
  float c = 8 * c0;
  c -= GetZ(vec2(-1, 1) * unitSize);
  c -= GetZ(vec2( 0, 1) * unitSize);
  c -= GetZ(vec2( 1, 1) * unitSize);
  c -= GetZ(vec2(-1, 0) * unitSize);
  c -= GetZ(vec2( 1, 0) * unitSize);
  c -= GetZ(vec2(-1,-1) * unitSize);
  c -= GetZ(vec2( 0,-1) * unitSize);
  c -= GetZ(vec2( 1,-1) * unitSize);
  c = 1 - smoothstep(0.0, 0.1, abs(c) / -c0);

  // �J���[���擾.
  fragColor = inColor * texture(texColor, inTexcoord);
  fragColor.rgb *= vec3(c);
}
