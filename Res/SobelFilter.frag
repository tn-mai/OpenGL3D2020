#version 450 core

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
  fragColor = inColor * texture(texColor, inTexcoord);

  // �|�X�^���[�[�V����.
  {
    float level = 5;
    float lum = max(fragColor.r, max(fragColor.g, fragColor.b));
    float quantized = lum + 0.5 / level;
    quantized = floor(quantized * level) / level;
    fragColor.rgb *= quantized / lum;
  }

  const int filterType = 1;

  // �\�[�x���t�B���^.
  if (filterType == 0) {
    vec2 unitSize = vec2(1) / vec2(textureSize(texDepth, 0));
    float c0 = GetZ(vec2(0));

    float x = 0;
    x -= 1 * GetZ(vec2(-unitSize.x, -unitSize.y));
    x -= 2 * GetZ(vec2(-unitSize.x,           0));
    x -= 1 * GetZ(vec2(-unitSize.x,  unitSize.y));
    x += 1 * GetZ(vec2( unitSize.x, -unitSize.y));
    x += 2 * GetZ(vec2( unitSize.x,           0));
    x += 1 * GetZ(vec2( unitSize.x,  unitSize.y));

    float y = 0;
    y -= 1 * GetZ(vec2(-unitSize.x,  unitSize.y));
    y -= 2 * GetZ(vec2(          0,  unitSize.y));
    y -= 1 * GetZ(vec2( unitSize.x,  unitSize.y));
    y += 1 * GetZ(vec2(-unitSize.x, -unitSize.y));
    y += 2 * GetZ(vec2(          0, -unitSize.y));
    y += 1 * GetZ(vec2( unitSize.x, -unitSize.y));

    float borderColor = sqrt(x * x + y * y) / -c0;
    borderColor = 1 - smoothstep(0.0, 0.5, borderColor);
    fragColor.rgb *= vec3(borderColor);
  }

  // 8�ߖT���v���V�A���t�B���^
  else if (filterType == 1) {
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
    fragColor.rgb *= vec3(c);
  }

  // ���v���V�A���I�u�K�E�V�A��(LoG)�t�B���^(�t�B���^�a ��=3)
  else if (filterType == 2) {
    vec2 unitSize = vec2(1) / vec2(textureSize(texDepth, 0));
    float c0 =  GetZ(vec2(0));
    float c = 16 * c0;

    c -= GetZ(vec2( 0, 2) * unitSize);

    c -= GetZ(vec2(-1, 1) * unitSize);
    c -= GetZ(vec2( 0, 1) * unitSize) * 2;
    c -= GetZ(vec2( 1, 1) * unitSize);

    c -= GetZ(vec2(-2, 0) * unitSize);
    c -= GetZ(vec2(-1, 0) * unitSize) * 2;
    c -= GetZ(vec2( 1, 0) * unitSize) * 2;
    c -= GetZ(vec2( 2, 0) * unitSize);

    c -= GetZ(vec2(-1,-1) * unitSize);
    c -= GetZ(vec2( 0,-1) * unitSize) * 2;
    c -= GetZ(vec2( 1,-1) * unitSize);

    c -= GetZ(vec2( 0,-2) * unitSize);

    c = 1 - smoothstep(0.0, 0.1, abs(c) / -c0);
    fragColor.rgb *= vec3(c);
  }
}

