/**
* @file Light.h
*/
#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED
#include "glad/glad.h"
#include "ShaderStorageBufferObject.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>

namespace Light {

// ��s�錾.
struct Frustum;
using FrustumPtr = std::shared_ptr<Frustum>;

FrustumPtr CreateFrustum(const glm::mat4& matProj, float zNear, float zFar);

/**
* �_����.
*/
struct Light
{
  // ���C�g�̎��.
  enum class Type {
    PointLight,       // �_����.
  };

  std::string name;   // ���C�g�̖��O.
  Type type = Type::PointLight; // ���C�g�̎��.
  glm::vec3 position; // ������ʒu.
  glm::vec3 color;    // ���C�g�̐F.
};
using LightPtr = std::shared_ptr<Light>;

/**
* ���C�g���Ǘ�����N���X.
*/
class LightManager
{
public:
  LightManager();
  ~LightManager() = default;
  LightManager(const LightManager&) = delete;
  LightManager& operator=(const LightManager&) = delete;

  LightPtr CreateLight(const glm::vec3& position, const glm::vec3& color);
  void RemoveLight(const LightPtr& light);
  LightPtr GetLight(size_t n) const;
  size_t LightCount() const;

  void Update(const glm::mat4& matView, const FrustumPtr& frustum);
  void Bind(GLuint binding) const;
  void Unbind(GLuint binding) const;

private:
  std::vector<LightPtr> lights;
  std::shared_ptr<ShaderStorageBufferObject> ssbo[2];
  int writingSsboNo = 0;
};

// ���C�g�}�l�[�W���N���X�̃|�C���^�^.
using LightManagerPtr = std::shared_ptr<LightManager>;

} // namespace Light

#endif//LIGHT_H_INCLUDED
