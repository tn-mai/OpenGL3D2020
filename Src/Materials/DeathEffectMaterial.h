/**
* @file DeathEffectMaterial.h
*/
#ifndef DEATHEFFECTMATERIAL_H_INCLUDED
#define DEATHEFFECTMATERIAL_H_INCLUDED
#include "../Material.h"

/**
* ���S�G�t�F�N�g.
*/
class DeathEffectMaterial : public Material
{
public:
  DeathEffectMaterial();
  virtual ~DeathEffectMaterial() = default;

  virtual void Update(float deltaTime) override;
  virtual void OnDraw() override;
  bool IsFinish() const;

private:
  std::shared_ptr<Texture::Image2D> texDeadEffect;
  float timer = 0;
  float speed = 0.5; // �G�t�F�N�g�i�s���x.
};

#endif // DEATHEFFECTMATERIAL_H_INCLUDED
