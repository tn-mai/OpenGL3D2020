#version 450

// ���͕ϐ�
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;
layout(location=2) in vec3 inPosition;
layout(location=3) in vec3 inNormal;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor;

// ���s����
struct DirectionalLight {
  vec4 direction;
  vec4 color;
};
layout(location=2) uniform DirectionalLight directionalLight;

// �_����
struct PointLight {
  vec4 position;
  vec4 color;
};
layout(location=4) uniform PointLight pointLight;

// ����
layout(location=6) uniform vec3 ambientLight;

// �t���O�����g�V�F�[�_�v���O����
void main()
{
  vec3 worldNormal = normalize(inNormal);
  vec3 totalLightColor = ambientLight;

  // ���s����
  {
    float theta = max(dot(worldNormal, -directionalLight.direction.xyz), 0);
    totalLightColor += directionalLight.color.rgb * theta;
  }

  // �_����
  {
    // �t���O�����g���烉�C�g�֌������x�N�g�����v�Z.
    vec3 lightVector = pointLight.position.xyz - inPosition;

    // �����ɂ�閾�邳�̕ω��ʂ��v�Z.
    float lengthSq = dot(lightVector, lightVector);
    float intensity = 1.0 / (1.0 + lengthSq);

    // �ʂ̌X���ɂ�閾�邳�̕ω��ʂ��v�Z.
    float theta = 1;
    if (lengthSq > 0) {
      vec3 direction = normalize(lightVector);
      theta = max(dot(worldNormal, direction), 0);
    }

    // �ω��ʂ��������킹�Ė��邳�����߁A���C�g�̖��邳�ϐ��ɉ��Z.
    totalLightColor += pointLight.color.rgb * theta * intensity;
  }

  fragColor = inColor * texture(texColor, inTexcoord);
  fragColor.rgb *= totalLightColor;
  //float a = clamp(inColor.a, 0, 1);
  //fragColor.a = smoothstep(1 - a, 1.2 - a, fragColor.a);
}

