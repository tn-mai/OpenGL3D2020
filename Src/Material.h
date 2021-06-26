/**
* @file Material.h
*/
#ifndef MATERIAL_H_INCLUDED
#define MATERIAL_H_INCLUDED
#include "Shader.h"
#include "Texture.h"
#include <memory>

/**
* アクターの質感.
*/
class Material
{
public:
  explicit Material(std::shared_ptr<Shader::Pipeline> p) : pipeline(p) {}
  virtual ~Material() = default;

  virtual void Update(float deltaTime) {}
  virtual void OnDraw() {}

  std::shared_ptr<Shader::Pipeline> pipeline;
};

#endif // MATERIAL_H_INCLUDED
