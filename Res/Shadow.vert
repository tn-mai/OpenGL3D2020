#version 450 core

// ���͕ϐ�
layout(location=0) in vec3 vPosition;
layout(location=2) in vec2 vTexcoord;
layout(location=4) in vec3 vMorphPosition;
layout(location=6) in vec3 vPrevBaseMeshPosition;
layout(location=8) in vec3 vPrevMorphTargetPosition;

// �o�͕ϐ�
layout(location=1) out vec2 outTexcoord;

out gl_PerVertex {
  vec4 gl_Position;
};

// ���j�t�H�[���ϐ�
layout(location=0) uniform mat4 matMVP;
layout(location=10) uniform vec3 morphWeight;

// ���_�V�F�[�_�v���O����
void main()
{
  // ���[�t�B���O
  vec3 curPosition = mix(vPosition, vMorphPosition, morphWeight.x);
  vec3 prevPosition = mix(vPrevBaseMeshPosition,
    vPrevMorphTargetPosition, morphWeight.y);
  vec3 position = mix(curPosition, prevPosition, morphWeight.z);

  outTexcoord = vTexcoord;
  gl_Position = matMVP * vec4(position, 1.0);
}
