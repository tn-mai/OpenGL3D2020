#version 450

// ���͕ϐ�
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;
layout(location=2) in vec3 inPosition;
layout(location=3) in vec3 inNormal;
layout(location=4) in vec3 inViewVector;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor;
layout(binding=1) uniform sampler2D texNormal;

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

const uvec2 screenSize = uvec2(1280, 720); // ��ʂ̑傫��.
const uvec2 tileCount = uvec2(8, 4); // ������̕�����.
const vec2 tileSize =
  vec2(screenSize) / vec2(tileCount); // �������̑傫��.
const uint maxLightCountInTile = 64; // ���Ɋ܂܂�郉�C�g�̍ő吔.
const uint maxLightCount = 1024; // �V�[���S�̂Ŏg���郉�C�g�̍ő吔.

/**
* ���C�g.
*/
struct Light {
  vec4 position; // ���C�g�̍��W.
  vec4 colorAndRange; // ���C�g�̐F(���邳)�ƁA���̓͂��͈�.
};

/**
* �`��Ɋ֌W���郉�C�g�̏��.
*/
layout(std430, binding=0) readonly restrict buffer TileData 
{
  uint lightIndices[tileCount.y][tileCount.x][maxLightCountInTile];
  uint lightCounts[tileCount.y][tileCount.x];
  Light lights[maxLightCount];
};

/**
* �@�����v�Z����.
*
* http://hacksoflife.blogspot.com/2009/11/per-pixel-tangent-space-normal-mapping.html
* http://www.thetenthplanet.de/archives/1180
*/
vec3 computeNormal()
{
  vec3 N = normalize(inNormal);
  vec3 normal = texture(texNormal, inTexcoord).rgb;
  if (dot(normal, normal) <= 0.01) {
    return N;
  }

  vec3 dp1 = dFdx(inViewVector);
  vec3 dp2 = dFdy(inViewVector);
  vec2 duv1 = dFdx(inTexcoord);
  vec2 duv2 = dFdy(inTexcoord);

  vec3 dp2perp = cross(dp2, N);
  vec3 dp1perp = cross(N, dp1);
  vec3 T = -dp2perp * duv1.x + dp1perp * duv2.x;
  vec3 B = dp2perp * duv1.y - dp1perp * duv2.y;

  // the transpose of texture-to-eye space matrix
  //float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
  //mat3 TBN = mat3(T * invmax, B * invmax, N);
  mat3 TBN = mat3(normalize(T), normalize(B), N);

  // transform the normal to eye space 
  normal = normal * 2 - 1;
  return TBN * normal;
}

// �t���O�����g�V�F�[�_�v���O����
void main()
{
  vec3 worldNormal = computeNormal();
  vec3 totalLightColor = ambientLight;
  vec3 totalSpecularColor = vec3(0);

  vec3 V = normalize(inViewVector);

  // ���s����
  {
    float theta = max(dot(worldNormal, -directionalLight.direction.xyz), 0);
    totalLightColor += directionalLight.color.rgb * theta;

    // �X�y�L����(Blinn-Phong).
    if(theta > 0) {
      const float specularExp = 50;
      vec3 H = normalize(-directionalLight.direction.xyz + V);
      float powNH = pow(max(dot(worldNormal, H), 0), specularExp) * ((specularExp + 1) / (2 * 3.14159267));
      float specular = 0.01 * powNH;
      totalSpecularColor += directionalLight.color.rgb * specular;
    }
  }

  // �_����
  uvec2 tileId = uvec2(gl_FragCoord.xy * (vec2(1) / tileSize));
  const uint lightCount = lightCounts[tileId.y][tileId.x];
  for (uint i = 0; i < lightCount; ++i) {
    uint lightIndex = lightIndices[tileId.y][tileId.x][i];
    if (lightIndex >= maxLightCount) {
      continue;
    }

    // �t���O�����g���烉�C�g�֌������x�N�g�����v�Z.
    vec3 lightVector = lights[lightIndex].position.xyz - inPosition;

    // �����ɂ�閾�邳�̕ω��ʂ��v�Z.
    float lengthSq = dot(lightVector, lightVector);
    float intensity = 1.0 / (1.0 + lengthSq);

    // �͈͊O�Ɍ����e�����Ȃ��悤�ɐ�������.
    // �����I�ɂ͕s���m(���̌����ǂꂾ���̋�������Œn���ɓ͂��Ă��邩���l����Ƃ悢)�����A�Q�[���ł͂�����ۂ���Ώ\���Ȃ̂�.
    // �Q�l: https://www.3dgep.com/forward-plus/
    const float fallOff = 0.75; // �������J�n���鋗��(�䗦).
    float rangeSq = lights[lightIndex].colorAndRange.a;
    rangeSq *= rangeSq;
    float attenuation = 1 - smoothstep(
      rangeSq * (fallOff * fallOff), rangeSq, lengthSq);
    intensity *= attenuation;

    // �ʂ̌X���ɂ�閾�邳�̕ω��ʂ��v�Z.
    float theta = 1;
    if (lengthSq > 0) {
      vec3 direction = normalize(lightVector);
      theta = max(dot(worldNormal, direction), 0);
    }

    // �ω��ʂ��������킹�Ė��邳�����߁A���C�g�̖��邳�ϐ��ɉ��Z.
    totalLightColor += lights[lightIndex].colorAndRange.rgb * theta * intensity;

    // �X�y�L����(Blinn-Phong).
    if(theta > 0) {
      const float specularExp = 50;
      vec3 H = normalize(lightVector + V);
      float powNH = pow(max(dot(worldNormal, H), 0), specularExp) * ((specularExp + 1) / (2 * 3.14159267));
      float specular = 0.1 * powNH;
      totalSpecularColor += lights[lightIndex].colorAndRange.rgb * specular * intensity;
    }
  }

  fragColor = inColor * texture(texColor, inTexcoord);
  fragColor.rgb *= totalLightColor;
  fragColor.rgb += totalSpecularColor;
#if 0
  fragColor.rgb = vec3(
    smoothstep(40, 64, float(lightCount)),
    smoothstep(20, 40, float(lightCount)),
    smoothstep(0, 20, float(lightCount)) + dot(fragColor.rgb, vec3(0.3, 0.6, 0.1)) * 0.1;
#endif
}

