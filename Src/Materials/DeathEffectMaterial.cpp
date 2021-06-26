/**
* @file DeathEffectMaterial.cpp
*/
#include "DeathEffectMaterial.h"
#include "../GameData.h"

/**
* コンストラクタ. 
*/
DeathEffectMaterial::DeathEffectMaterial() :
  Material(GameData::Get().pipelineDeathEffect),
  texDeadEffect(Texture::CreateImage2D("Res/DeathEffect.tga", Texture::ImageType::non_color))
{
}

/**
* 死亡エフェクトの更新.
*
* @param deltaTime 前回の更新からの経過時間(秒).
*/
void DeathEffectMaterial::Update(float deltaTime)
{
  timer += deltaTime * speed;
}

/**
* 描画前処理. 
*/
void DeathEffectMaterial::OnDraw()
{
  pipeline->SetEffectTimer(timer);
  texDeadEffect->Bind(3);
}

/**
* エフェクトが終了したか調べる.
*
* @retval true  終了している.
* @retval false まだ実行中.
*/
bool DeathEffectMaterial::IsFinish() const
{
  return timer >= 1.0f;
}

