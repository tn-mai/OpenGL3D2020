#version 450 core

// ���͕ϐ�
layout(location=0) in vec3 vPosition;
//layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexcoord;
//layout(location=3) in vec3 vNormal;

// �o�͕ϐ�
layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexcoord;

out gl_PerVertex {
  vec4 gl_Position;
};

// ���j�t�H�[���ϐ�
layout(location=0) uniform mat4 matMVP;
layout(location=1) uniform mat4 matModel;
layout(location=2) uniform vec4 objectColor;

// ���_�V�F�[�_�v���O����
void main()
{
  outColor = objectColor;
  outTexcoord = vTexcoord;
  gl_Position = matMVP * vec4(vPosition, 1.0);
}