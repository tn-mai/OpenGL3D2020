/**
* @file DeathEffectMaterial.cpp
*/
#include "DeathEffectMaterial.h"
#include "../GameData.h"

/**
* �R���X�g���N�^. 
*/
DeathEffectMaterial::DeathEffectMaterial() :
  Material(GameData::Get().pipelineDeathEffect),
  texDeadEffect(Texture::CreateImage2D("Res/DeathEffect.tga", Texture::ImageType::non_color))
{
}

/**
* ���S�G�t�F�N�g�̍X�V.
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b).
*/
void DeathEffectMaterial::Update(float deltaTime)
{
  timer += deltaTime * speed;
}

/**
* �`��O����. 
*/
void DeathEffectMaterial::OnDraw()
{
  pipeline->SetEffectTimer(timer);
  texDeadEffect->Bind(3);
}

/**
* �G�t�F�N�g���I�����������ׂ�.
*
* @retval true  �I�����Ă���.
* @retval false �܂����s��.
*/
bool DeathEffectMaterial::IsFinish() const
{
  return timer >= 1.0f;
}

