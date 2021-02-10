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
layout(binding=1) uniform sampler2D texNormal;
layout(binding=2) uniform sampler2D texMetallicSmoothness;

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

// ���_���W
layout(location=7) uniform vec3 viewPosition;

const uvec2 screenSize = uvec2(1280, 720); // ��ʂ̑傫��.
const uvec2 tileCount = uvec2(8, 4); // ������̕�����.
const vec2 tileSize =
  vec2(screenSize) / vec2(tileCount); // �������̑傫��.
const uint maxLightCountInTile = 64; // ���Ɋ܂܂�郉�C�g�̍ő吔.
const uint maxLightCount = 1024; // �V�[���S�̂Ŏg���郉�C�g�̍ő吔.

// ���C�g�̎��.
const float Type_PointLight = 0;
const float Type_SpotLight = 1;

const float pi = 3.14159265358979323846264338327950288;

/**
* ���C�g.
*/
struct Light {
  vec4 positionAndType;     // ���C�g�̍��W�Ǝ��.
  vec4 colorAndRange;       // ���C�g�̐F(���邳)�ƁA���̓͂��͈�.
  vec4 direction;           // ���C�g���Ƃ炷����.
  vec4 coneAndFalloffAngle; // �X�|�b�g���C�g���Ƃ炷�p�x�ƌ����J�n�p�x.
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
vec3 computeNormal(vec3 V)
{
  vec3 N = normalize(inNormal);
  vec3 normal = texture(texNormal, inTexcoord).rgb;
  if (dot(normal, normal) <= 0.01) {
    return N;
  }

  vec3 dp1 = dFdx(V);
  vec3 dp2 = dFdy(V);
  vec2 duv1 = dFdx(inTexcoord);
  vec2 duv2 = dFdy(inTexcoord);

  vec3 dp2perp = cross(dp2, N);
  vec3 dp1perp = cross(N, dp1);
  vec3 T = -dp2perp * duv1.x + dp1perp * duv2.x;
  vec3 B = dp2perp * duv1.y - dp1perp * duv2.y;

  // the transpose of texture-to-eye space matrix
  //float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
  //mat3 TBN = mat3(T * invmax, B * invmax, N);
  T = normalize(cross(B, N));
  mat3 TBN = mat3(normalize(T), normalize(B), N);

  // transform the normal to eye space 
  normal = normal * 2 - 1;
  return TBN * normal;
}

/**
* �t���l�����˗����v�Z����.
*
* @param F0  �p�x��0�x�̂Ƃ��̃t���l�����˗�.
* @param L   ���̓��˕���.
* @param V   �����x�N�g��.
*/
float Fresnel(float F0, vec3 L, vec3 V)
{
  // 8.656170 -> https://seblagarde.wordpress.com/2011/08/17/hello-world/
  // �n�[�t�x�N�g���̒�`����Adot(direction, H)��dot(V, H)�͓������ʂɂȂ�.
  // ���_���猩���}�C�N���t�@�Z�b�g�@���̕��ς̓n�[�t�x�N�g���Ɠ�����.
  // dot(V, N)�łȂ�dot(V, H)���g�����ƂŃ}�C�N���t�@�Z�b�g���l�������t���l������������.
  vec3 H = normalize(L + V);
  return F0 + (1 - F0) * exp2(-8.656170 * max(dot(V, H), 0));
}

/**
* �X�y�L�������v�Z����(Blinn-Phong).
*/
float computeSpecular(vec3 direction, vec3 worldNormal, vec3 V, float shininess, float normalizationFactor)
{
  vec3 H = normalize(direction + V);

  // Blinn-Phong BRDF�̐��K���W���ɂ��Ă̋c�_�͈ȉ��̃T�C�g���Q��.
  // http://www.rorydriscoll.com/2009/01/25/energy-conservation-in-games/
  // ��L�T�C�g�̃R�����g�ɂ��ƁA���K���W��"(n+8)/8��"�́A�^�̐��K���W��"(n+2)(n+4)/8��(2^(-n/2)+n)"�̌����I�ȋߎ��ł���炵��.
  // ���K���W���̓��o�͂��� -> http://www.farbrausch.de/~fg/stuff/phong.pdf
  // �Ȃ�"(n+2)/2��"��Phong BRDF�̐��K���W���Ȃ̂����A�����̃T�C�g�������Blinn-Phong�ɓK�p���Ă���悤���B
  //
  // Tri-Ace�ɂ�铯�l�̓��o�ł͐��K���W����"(n+2)/4��(2-2^(-n/2)"�Ƃ��A�ߎ���"(n+2.04)/8��"�Ƃ��Ă���.
  // http://research.tri-ace.com/Data/BasicOfReflectance.ppt
  // �����Blinn-Phong�̐��K���W���ł���ABlinn-Phong BRDF�̐��K���W���ł͂Ȃ��_�ɒ���.
  // Phong��Blinn-Phong��BRDF���ǂ����Ő��K���W�����قȂ�̂Œ���.
  // BRDF�o�[�W������Phong�܂���Blinn-Phong�v�Z�̂��ƁA�����dot(N, L)���|����.
  // ���̃R�[�h�ł͂��̊֐��̊O�ōs���Ă���.
#if 1
  // Normalized Blinn-Phong BRDF
  // http://www0.cs.ucl.ac.uk/staff/j.kautz/GameCourse/04_PointLights.pdf
  return pow(max(dot(worldNormal, H), 0), shininess) * normalizationFactor;
#else
  // Velvet Assassin�G���W���ō̗p���ꂽ�A�t���l������"dot(L,H)^-3"�ŋߎ�������@.
  // F0�����݂��Ȃ����߁A����x�����̂܂܃X�y�L�����W���ɂ��Ă��܂��Ƃ₽����򊴂��o��.
  // http://www.thetenthplanet.de/archives/255
  float LdotH = pow(max(dot(direction, H), 0), 3);
  return pow(max(dot(worldNormal, H), 0), shininess) * (shininess + 1) / (8.0 * pi * LdotH);
#endif
}

// �t���O�����g�V�F�[�_�v���O����
void main()
{
  float metallic = texture(texMetallicSmoothness, inTexcoord).r; // 0=����� 1=����
  float smoothness = texture(texMetallicSmoothness, inTexcoord).g;
  float shininess = exp2(smoothness * 10 + 3);
  //smoothness *= 0.5; // VA�G���W���̎����g���ꍇ�̌W��. ���l�͓K���ɂ�����ۂ��Ȃ�悤�ɑI��.

  // ���K���W�����v�Z.
  float normalizationFactor = (shininess + 8) * (1.0 / (8.0 * pi));

  // �����Ɩ@������v(�p�x��0)����ꍇ�̃t���l���W��.
  // �����(�U�d��)��F0��2%�`6%���x�Ɏ��܂�. �������A��Ηނ�10�`20%�ɒB����ꍇ������.
  // ����(����)��F0��50%�`100%���x��RGB���ƂɈ���Ă���. �Ⴆ�΋���(1.0, 0.71, 0.29)�Ȃ�.
  const float ior = 1.5;
  float f0 = mix(0.04, 1.0, metallic);//pow((ior - 1) / (ior + 1), 2);
  // ����x��F0�ɓ]�p����o�[�W����.
  //f0 = smoothness;

  vec3 viewVector = normalize(viewPosition - inPosition);
  vec3 worldNormal = computeNormal(viewVector);
  vec3 totalLightColor = ambientLight;
  vec3 totalSpecularColor = vec3(0);

  // ���s����
  {
    float F = Fresnel(f0, -directionalLight.direction.xyz, viewVector);

    float theta = max(dot(worldNormal, -directionalLight.direction.xyz), 0);
    totalLightColor += directionalLight.color.rgb * theta * (1 - F);

    totalSpecularColor += directionalLight.color.rgb * theta *
      computeSpecular(-directionalLight.direction.xyz, worldNormal, viewVector, shininess, normalizationFactor) * F;
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
    vec3 lightVector = lights[lightIndex].positionAndType.xyz - inPosition;

    // �����ɂ�閾�邳�̕ω��ʂ��v�Z.
    float lengthSq = dot(lightVector, lightVector);
    float intensity = 1.0 / (1.0 + lengthSq);

    lightVector = normalize(lightVector);

    // �͈͊O�Ɍ����e�����Ȃ��悤�ɐ�������.
    // �����I�ɂ͕s���m(���̌����ǂꂾ���̋�������Œn���ɓ͂��Ă��邩���l����Ƃ悢)�����A�Q�[���ł͂�����ۂ���Ώ\���Ȃ̂�.
    // �Q�l: https://www.3dgep.com/forward-plus/
    const float fallOff = 0.75; // �������J�n���鋗��(�䗦).
    float rangeSq = lights[lightIndex].colorAndRange.a;
    rangeSq *= rangeSq;
    float attenuation = smoothstep(rangeSq,
      rangeSq * (fallOff * fallOff), lengthSq);
    if (attenuation <= 0) {
      continue;
    }
    intensity *= attenuation;

    // �X�|�b�g���C�g�̏ꍇ�A�~���O�����̌������v�Z����.
    if (lights[lightIndex].positionAndType.w == Type_SpotLight) {
      vec3 direction = lights[lightIndex].direction.xyz;
      float coneAngle = lights[lightIndex].coneAndFalloffAngle.x;
      // ���C�g����t���O�����g�֌������x�N�g���ƁA�X�|�b�g���C�g�̃x�N�g���̂Ȃ��p��
      // ���C�g���Ƃ炷�p�x�ȏ�Ȃ�͈͊O.
      // cos�ƂŔ�r���Ă��邽�߁A�s�������t�ɂȂ邱�Ƃɒ���.
      float angle = dot(direction, -lightVector);
      if (angle <= coneAngle) {
        continue;
      }
      // �����J�n�p�x�ƊO���p�x�̊Ԃŕ��.
      float falloffAngle = lights[lightIndex].coneAndFalloffAngle.y;
      intensity *= 1 - smoothstep(falloffAngle, coneAngle, angle);
      if (intensity <= 0) {
        continue;
      }
    }

    // �ʂ̌X���ɂ�閾�邳�̕ω��ʂ��v�Z.
    float theta = 1;
    if (lengthSq > 0) {
      theta = max(dot(worldNormal, lightVector), 0);
    }

    float F = Fresnel(f0, -directionalLight.direction.xyz, viewVector);

    // �ω��ʂ��������킹�Ė��邳�����߁A���C�g�̖��邳�ϐ��ɉ��Z.
    totalLightColor += lights[lightIndex].colorAndRange.rgb * theta * intensity * (1 - F);

    totalSpecularColor += lights[lightIndex].colorAndRange.rgb * theta * intensity *
      computeSpecular(lightVector, worldNormal, viewVector, shininess, normalizationFactor) * F;
  }

  fragColor = inColor * texture(texColor, inTexcoord);
  vec3 diffuse  = fragColor.rgb * (1 - metallic) * (1 / pi);
  vec3 specular = mix(vec3(1), fragColor.rgb, metallic);
  fragColor.rgb = (diffuse * totalLightColor) + (specular * totalSpecularColor) +
    (ambientLight.rgb * fragColor.rgb * metallic);

  //fragColor.rgb = texture(texNormal, inTexcoord).rgb;
  //fragColor.rgb = fragColor.rgb * 0.0000001 + (worldNormal * 0.5 + 0.5);
  //fragColor.rgb = fragColor.rgb * 0.0001 + vec3((shininess - 2) / 48);
#if 0
  fragColor.rgb = vec3(
    smoothstep(40, 64, float(lightCount)),
    smoothstep(20, 40, float(lightCount)),
    smoothstep(0, 20, float(lightCount)) + dot(fragColor.rgb, vec3(0.3, 0.6, 0.1)) * 0.1;
#endif
}

