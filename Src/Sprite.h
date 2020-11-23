/**
* @file Sprite.h
*/
#ifndef SPRITE_H_INCLUDED
#define SPRITE_H_INCLUDED
#include "Texture.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>

/**
* �X�v���C�g.
*/
class Sprite
{
public:
  Sprite() = default;
  Sprite(const glm::vec3& pos, std::shared_ptr<Texture::Image2D> tex,
    const glm::vec2& origin = glm::vec2(0),
    const glm::vec2& size = glm::vec2(1));
  
  void Update(float deltaTime);

  std::shared_ptr<Texture::Image2D> texture; // �摜���܂ރe�N�X�`��.
  glm::vec2 imageOrigin = glm::vec2(0);      // �摜�̍����e�N�X�`�����W.
  glm::vec2 imageSize = glm::vec2(1);        // �摜�̑傫��.

  glm::vec3 position = glm::vec3(0); // ���W.
  float rotation = 0;                // Z����].
  glm::vec2 scale =glm::vec2(1);     // �g�嗦.
  glm::vec4 color = glm::vec4(1);    // �F�ƕs�����x.

  glm::vec3 velocity = glm::vec3(0); // ���x.
  float angularVelocity = 0;
  glm::vec2 scaleVelocity = glm::vec2(0);
  glm::vec4 colorVelocity = glm::vec4(0);

  float gravityScale = 0; // �d�͂̉e����.
  float lifespan = 0;     // ����.
  bool isDead = false;    // ���S�t���O.

};

void UpdateSpriteList(std::vector<std::shared_ptr<Sprite>>&, float);

/**
* �X�v���C�g�`��N���X.
*/
class SpriteRenderer
{
public:
  SpriteRenderer() = default;
  ~SpriteRenderer();
  SpriteRenderer(const SpriteRenderer&) = delete;
  SpriteRenderer& operator=(const SpriteRenderer&) = delete;

  // �������Ǘ�.
  bool Allocate(size_t maxSpriteCount);
  void Free();

  void Update(const std::vector<std::shared_ptr<Sprite>>& sprites, const glm::mat4& matView);
  void Draw(std::shared_ptr<Shader::Pipeline> pipeline, const glm::mat4& matVP) const;

private:
  GLuint ibo = 0;
  struct Buffer {
    GLuint vboPosition = 0;
    GLuint vboColor = 0;
    GLuint vboTexcoord = 0;
    GLuint vboNormal = 0;
    GLuint vao = 0;
  };
  Buffer buffers[2];
  size_t updatingBufferIndex = 0;

  size_t maxSpriteCount = 0; // �i�[�ł���ő�X�v���C�g��.

  struct Primitive {
    GLsizei count;
    GLint baseVertex;
    std::shared_ptr<Texture::Image2D> texture;
  };
  std::vector<Primitive> primitives;
};

#endif // SPRITE_H_INCLUDED
