/**
* @file Actor.h
*/
#ifndef ACTOR_H_INCLUDED
#define ACTOR_H_INCLUDED
#include "glad/glad.h"
#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>

/**
* �A�j���[�V�����f�[�^.
*/
struct Animation
{
  std::vector<const Mesh::Primitive*> list; // �A�j���[�V�����Ɏg���v���~�e�B�u�̃��X�g.
  float interval = 0.3f; // �v���~�e�B�u��؂�ւ���Ԋu(�b).
  bool isLoop = true; // ���[�v�t���O.
};

/**
* �Փ˔���.
*/
struct Collision
{
  // �Փˌ`��̎��.
  enum Shape {
    none,     // �Փ˔���Ȃ�.
    cylinder, // �~��.
    box,      // ������.
  };
  Shape shape = Shape::none;
  bool blockOtherActors = true; // �ʂ蔲�����Ȃ��Ȃ�true�A��������Ȃ�false.

  // �~���̃p�����[�^.
  float top;    // �~���̏�[.
  float bottom; // �~���̉��[.
  float radius; // �~���̔��a.

  // �����̂̃p�����[�^.
  glm::vec3 boxMin;
  glm::vec3 boxMax;
};

/**
* �����s���E�{�b�N�X.
*/
struct AABB
{
  glm::vec3 c = glm::vec3(0); // ���S���W.
  glm::vec3 r = glm::vec3(0); // �e���̔��a.
};

/**
* �A�N�^�[.
*/
class Actor
{
public:
  Actor() = default;
  virtual ~Actor() = default;
  Actor(std::string actorName, const Mesh::Primitive* prim,
    std::shared_ptr<Texture::Image2D> tex, const glm::vec3& pos);

  void Update(float deltTIme);
  virtual void OnUpdate(float) {}
  virtual void OnDestroy() {}

  // �`��̎��.
  enum DrawType {
    color,  // �ʏ�`��.
    shadow, // �e�`��.
  };
  void Draw(const Shader::Pipeline& pipeline, const glm::mat4& matVP, DrawType) const;

  void SetCylinderCollision(float top, float bottom, float radius);
  void SetBoxCollision(const glm::vec3& min, const glm::vec3& max);
  void SetAnimation(
    std::shared_ptr<Animation> animation);

  std::string name; // �A�N�^�[�̖��O.

  // �A�N�^�[�̓�����.
  enum State {
    idle, // �������Ă��Ȃ�(�ҋ@��).
    run,  // �����Ă���.
    search, // ���G���Ă���.
    attack, // �U�����Ă���.
    damage, // �_���[�W���󂯂Ă���.
    down, // �|��Ă���.
    dead, // ����ł���.
    blow_off, // �������ł���.
  };
  State state = State::idle; // ���݂̓�����.
  float health = 0; // �ϋv��.
  float lifespan = 0; // ����(�b). 0�ȉ��͖�������.
  float timer = 0;  // �ėp�^�C�}�[.

  const Mesh::Primitive* primitive = nullptr;
  const Mesh::Primitive* morphTarget = nullptr;
  const Mesh::Primitive* prevBaseMesh = nullptr;
  const Mesh::Primitive* prevMorphTarget = nullptr;
  float prevMorphWeight = 0;
  float morphTransitionTimer = 0;
  std::shared_ptr<Texture::Image2D> texture;
  std::shared_ptr<Texture::Image2D> texNormal;
  std::shared_ptr<Texture::Image2D> texMetallicSmoothness;
  std::shared_ptr<Texture::Sampler> samplers[3];

  glm::vec3 position = glm::vec3(0); // �A�N�^�[�̕\���ʒu.
  glm::vec3 rotation = glm::vec3(0); // �A�N�^�[�̌���.
  glm::vec3 scale = glm::vec3(1); // �A�N�^�[�̑傫��.
  glm::vec3 velocity = glm::vec3(0); // �A�N�^�[�̈ړ����x.
  glm::vec4 baseColor = glm::vec4(1); // �A�N�^�[�̐F.

  // �A�j���[�V�����p�f�[�^.
  std::shared_ptr<Animation> animation; // �A�j���[�V�����f�[�^.
  size_t animationNo = 0; // �\������v���~�e�B�u�̔ԍ�.
  float animationTimer = 0; // �v���~�e�B�u�؂�ւ��^�C�}�[(�b).

  // �Փ˔���p�̕ϐ�.
  Collision collision;
  AABB boundingBox;

  // �Փˉ����֐��ւ̃|�C���^.
  void (*OnHit)(Actor&, Actor&) = [](Actor&, Actor&) {};

  std::shared_ptr<Actor> attackActor; // �U���̏Փ˔���p�A�N�^�[.

  float gravityScale = 0; // �d�͉e����.
  float friction = 0.7f;  // ���C�W��.
  float drag = 0;         // �����������C��.

  bool isDead = false; // ���S�t���O.
  bool isShadowCaster = true;
};

using ActorPtr = std::shared_ptr<Actor>; // �A�N�^�[�|�C���^�^.
using ActorList = std::vector<ActorPtr>; // �A�N�^�[�z��^.

void UpdateActorList(ActorList& actorList, float deltaTime);
void RenderActorList(const ActorList& actorList,
  const glm::mat4& matVP, Actor::DrawType drawType);

bool DetectCollision(Actor&, Actor&, bool block);

/**
* ����.
*/
struct Segment
{
  glm::vec3 start; // �����̎n�_.
  glm::vec3 end;   // �����̏I�_.
};

/**
* ��.
*/
struct Sphere
{
  glm::vec3 center; // ���̒��S���W.
  float radius;     // ���̔��a.
};

/**
* �~��.
*/
struct Cone
{
  glm::vec3 tip; // �~���̒��_���W.
  float height;  // �~���̍���.
  glm::vec3 direction; // �~���̌���.
  float radius;  // �~���̒�ʂ̔��a.
};

/**
* ����.
*/
struct Plane
{
  glm::vec3 point;  // ���ʏ�̔C�ӂ̍��W.
  glm::vec3 normal; // ���ʂ̖@��.
};

bool Intersect(const Segment& seg, const Plane& plane, glm::vec3* p);
bool SphereInsidePlane(const Sphere& sphere, const Plane& plane);
bool ConeInsidePlane(const Cone& cone, const Plane& plane);

#endif // ACTOR_H_INCLUDED
