#version 450

// ���͕ϐ�
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor;
layout(binding=1) uniform sampler2D texHatching;

// �������l.
const float threshold = 0.25;

// �t���O�����g�V�F�[�_�v���O����
void main()
{
  // �J���[���擾.
  fragColor = inColor * texture(texColor, inTexcoord);
  vec3 hatching = texture(texHatching, gl_FragCoord.xy * (1.0 / 64)).rgb;

  // �ł����邢�v�f��S�̖̂��邳�Ƃ���.
  // YUV�Ȃǂ̋P�x�v�Z�ł͐��قڎ���ł��܂����߁A�e�̔���ɂ͌����Ă��Ȃ�.
  float brightness = max(fragColor.r, max(fragColor.g, fragColor.b));

  // 
  fragColor.rgb *= mix(hatching, vec3(1), smoothstep(0, threshold, brightness));
}
