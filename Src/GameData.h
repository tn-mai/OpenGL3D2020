/**
* @file GameData.h
*/
#ifndef GAMEDATA_H_INCLUDED
#define GAMEDATA_H_INCLUDED
#include "glad/glad.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Actor.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <random>

/**
* �Q�[���S�̂Ŏg���f�[�^.
*/
class GameData
{
public:
  static GameData& Get();
  bool Initialize(GLFWwindow*);
  void Update();

  /**
   �L�[�̎��.

   ������Ă���L�[�̔���̓r�b�g���Z�q���g����:
     const uint32_t keyPressed = GameData::Get().keyPressed;
     if (keyPressed & GameData::Key::enter) {
       // Enter�L�[��������Ă���ꍇ�̏���.
     }
   �̂悤�ɂ���.
   �����ꂽ�u�Ԃ�m�肽���ꍇ��keyPressedInLastFrame���g��.
  */
  enum Key {
    enter      = 0b0000'0000'0000'0001, // Enter�L�[
    left       = 0b0000'0000'0000'0010, // ���L�[(��)
    right      = 0b0000'0000'0000'0100, // ���L�[(�E)
    up         = 0b0000'0000'0000'1000, // ���L�[(��)
    down       = 0b0000'0000'0001'0000, // ���L�[(��)
    shot       = 0b0000'0000'0010'0000, // �e���˃L�[
    jump       = 0b0000'0000'0100'0000, // �W�����v�L�[.
    grenade    = 0b0000'0000'1000'0000, // ��֒e�L�[.
    build      = 0b0000'0001'0000'0000, // ���z�L�[.
    scrollup   = 0b0001'0000'0000'0000, // �}�E�X�z�C�[����.
    scrolldown = 0b0010'0000'0000'0000, // �}�E�X�z�C�[����.
  };
  uint32_t keyPressed = 0; // �����Ă���L�[.
  uint32_t keyPressedInLastFrame = 0; // �Ō�̃t���[���ŉ����ꂽ�L�[.

  // �}�E�X�J�[�\���̍��W.
  glm::vec2 cursorPosition = glm::vec2(0);

  // �v���~�e�B�u�ԍ�.
  // �v���~�e�B�u�̓ǂݍ��ݏ��ƈ�v�����邱��.
  enum PrimNo {
    ground,
    tree,
    house,
    cube,
    plane,
    bullet,
    m67_grenade,
    wooden_barrior,
    zombie_male_walk_0,
    zombie_male_walk_1,
    zombie_male_walk_2,
    zombie_male_walk_3,
    zombie_male_walk_4,
    zombie_male_walk_5,
    zombie_male_down_0,
    zombie_male_down_1,
    zombie_male_down_2,
    zombie_male_down_3,
    zombie_male_attack_0,
    zombie_male_attack_1,
    zombie_male_attack_2,
    zombie_male_attack_3,
    zombie_male_attack_4,
    zombie_male_attack_5,
    zombie_male_attack_6,
    zombie_male_damage_0,
    zombie_male_damage_1,
    zombie_male_damage_2,
    zombie_male_damage_3,

    player_idle_0,
    player_idle_1,
    player_idle_2,
    player_run_front_0,
    player_run_front_1,
    player_run_front_2,
    player_run_front_3,
    player_run_front_4,
    player_run_front_5,

    player_run_back_0,
    player_run_back_1,
    player_run_back_2,
    player_run_back_3,
    player_run_back_4,
    player_run_back_5,

    player_run_left_0,
    player_run_left_1,
    player_run_left_2,
    player_run_left_3,
    player_run_left_4,
    player_run_left_5,

    player_run_right_0,
    player_run_right_1,
    player_run_right_2,
    player_run_right_3,
    player_run_right_4,
    player_run_right_5,

    player_down_0,
    player_down_1,
    player_down_2,
    player_down_3,

    player_damage_0,
    player_damage_1,
    player_damage_2,
  };
  void Draw(PrimNo) const;

  std::shared_ptr<Shader::Pipeline> pipeline = nullptr;
  std::shared_ptr<Shader::Pipeline> pipelineSimple;
  std::shared_ptr<Shader::Pipeline> pipelineShadow;
  std::shared_ptr<Shader::Pipeline> pipelineSobelFilter;
  std::shared_ptr<Shader::Pipeline> pipelineGaussianFilter;
  std::shared_ptr<Shader::Pipeline> pipelinePosterization;
  std::shared_ptr<Shader::Pipeline> pipelineHatching;
  std::shared_ptr<Shader::Pipeline> pipelineOutline;
  Mesh::PrimitiveBuffer primitiveBuffer;
  Texture::Sampler sampler;
  Texture::Sampler samplerClampToEdge;
  GLFWwindow* window = nullptr;

  std::mt19937 random;

  // �A�j���[�V�����f�[�^.
  std::shared_ptr<Animation> anmZombieMaleWalk;
  std::shared_ptr<Animation> anmZombieMaleDown;
  std::shared_ptr<Animation> anmZombieMaleAttack;
  std::shared_ptr<Animation> anmZombieMaleDamage;

  std::shared_ptr<Animation> anmBigZombieWalk;
  std::shared_ptr<Animation> anmBigZombieDamage;
  std::shared_ptr<Animation> anmBigZombieDown;
  std::shared_ptr<Animation> anmBigZombieAttack;

  std::shared_ptr<Animation> anmPlayerIdle;
  std::shared_ptr<Animation> anmPlayerRunFront;
  std::shared_ptr<Animation> anmPlayerRunBack;
  std::shared_ptr<Animation> anmPlayerRunLeft;
  std::shared_ptr<Animation> anmPlayerRunRight;
  std::shared_ptr<Animation> anmPlayerDown;
  std::shared_ptr<Animation> anmPlayerDamage;

  std::shared_ptr<Texture::Image2D> texBlood;
  std::shared_ptr<Texture::Image2D> texHatching;

  std::shared_ptr<Texture::Image2D> texGroundNormal;
  std::shared_ptr<Texture::Image2D> texZombieNormal;

  // �|�����]���r�̐�.
  size_t killCount = 0;

  // �d��.
  glm::vec3 gravity = glm::vec3(0, -9.8f, 0);

  // �X�N���[���l.
  double prevScroll = 0;
  double curScroll = 0;
  double tmpScroll = 0;

private:
  GameData() = default;
  ~GameData();
  GameData(const GameData&) = delete;
  GameData& operator=(const GameData&) = delete;

  std::shared_ptr<Animation> LoadAnimation(const char* filename, size_t size, float interval, bool isLoop);
};


#endif // GAMEDATA_H_INCLUDED
