/**
* @file Light.cpp
*/
#include "Light.h"
#include "Actor.h"
#include <iostream>

namespace Light {

const glm::uint screenSizeX = 1280; // ��ʂ̕�.
const glm::uint screenSizeY = 720; // ��ʂ̍���.
const glm::uint tileCountX = 8; // �������X�����̕�����.
const glm::uint tileCountY = 4; // �������Y�����̕�����.

// �������̕�.
const float tileSizeX =
  (float(screenSizeX) / float(tileCountX));

// �������̍���.
const float tileSizeY =
  (float(screenSizeY) / float(tileCountY));

const glm::uint maxLightCountInTile = 64; // ���Ɋ܂܂�郉�C�g�̍ő吔.
const glm::uint maxLightCount = 1024; // �V�[���S�̂Ŏg���郉�C�g�̍ő吔.

/**
* �\�����郉�C�g��I�ʂ��邽�߂̃T�u������.
*/
struct SubFrustum
{
  Plane planes[4]; // left, right, top, bottom

  // for AABB
  glm::vec3 aabbCenter;
  glm::vec3 aabbHalfSize;
};

/**
* �\�����郉�C�g��I�ʂ��邽�߂̎�����.
*/
struct Frustum
{
  SubFrustum baseFrustum;
  float zNear;
  float zFar;
  SubFrustum tiles[tileCountY][tileCountX];
};

/**
* �V�F�[�_�Ɠ����`���̃��C�g�f�[�^.
*/
struct LightForShader
{
  glm::vec4 positionAndType;
  glm::vec4 colorAndRange;
  glm::vec4 direction;
  glm::vec4 coneAndFalloffAngle;
};

/**
* �`��Ɋ֌W���郉�C�g�̏��.
*/
struct LightData
{
  glm::uint lightIndices[tileCountY][tileCountX][maxLightCountInTile];
  glm::uint lightCounts[tileCountY][tileCountX];
  LightForShader lights[maxLightCount];
};

/**
* �X�N���[�����W���r���[���W�ɕϊ�����.
*
* @param p          �X�N���[�����W.
* @param matInvProj �t�v���W�F�N�V�����s��.
*
* @return �r���[���W�n�ɕϊ�����p�̍��W.
*/
glm::vec3 ScreenToView(const glm::vec3& p, const glm::mat4& matInvProj)
{
  glm::vec4 pp;
  pp.x = (p.x / screenSizeX) * 2 - 1;
  pp.y = (p.y / screenSizeY) * 2 - 1;
  pp.z = p.z;
  pp.w = 1;
  pp = matInvProj * pp;
  pp /= pp.w;
  return pp;
}

/**
* �r���[���W�n�̃T�u��������쐬����.
*
* @param matProj �v���W�F�N�V�����s��.
* @param x0      �^�C���̍�����X���W. 
* @param y0      �^�C���̍�����Y���W. 
* @param x1      �^�C���̉E���X���W. 
* @param y1      �^�C���̉E���Y���W. 
*
* @return �쐬�����T�u������.
*/
SubFrustum CreateSubFrustum(const glm::mat4& matInvProj,
  float x0, float y0, float x1, float y1, float zNear, float zFar)
{
  const glm::vec3 viewPos0 = ScreenToView(glm::vec3(x0, y0, 1), matInvProj);
  const glm::vec3 viewPos1 = ScreenToView(glm::vec3(x1, y0, 1), matInvProj);
  const glm::vec3 viewPos2 = ScreenToView(glm::vec3(x1, y1, 1), matInvProj);
  const glm::vec3 viewPos3 = ScreenToView(glm::vec3(x0, y1, 1), matInvProj);
  const glm::vec3 p = glm::vec3(0);

  SubFrustum f;
  f.planes[0] = Plane{ p, glm::normalize(glm::cross(viewPos0, viewPos3)) };
  f.planes[1] = Plane{ p, glm::normalize(glm::cross(viewPos2, viewPos1)) };
  f.planes[2] = Plane{ p, glm::normalize(glm::cross(viewPos3, viewPos2)) };
  f.planes[3] = Plane{ p, glm::normalize(glm::cross(viewPos1, viewPos0)) };

  const glm::vec3 viewPos0Near = viewPos0 * (zNear / zFar);
  const glm::vec3 viewPos2Near = viewPos2 * (zNear / zFar);
  const float xLeft = std::min(viewPos0.x, viewPos0Near.x);
  const float xRight = std::max(viewPos2.x, viewPos2Near.x);
  const float yBottom = std::min(viewPos0.y, viewPos0Near.y);
  const float yTop = std::max(viewPos2.y, viewPos2Near.y);
  f.aabbHalfSize = glm::vec3(xRight - xLeft, yTop - yBottom, zFar - zNear) * 0.5f;
  f.aabbCenter = glm::vec3(xRight + xLeft, yTop + yBottom, -(zFar + zNear)) * 0.5f;

  return f;
}

/**
* �r���[���W�n�̎�������쐬����.
*
* @param matProj �v���W�F�N�V�����s��.
* @param zNear   ���_����߃N���b�v���ʂ܂ł̋���.
* @param zFar    ���_���牓�N���b�v���ʂ܂ł̋���.
*
* @return �쐬����������.
*/
FrustumPtr CreateFrustum(const glm::mat4& matProj, float zNear, float zFar)
{
  FrustumPtr frustum = std::make_shared<Frustum>();

  const glm::mat4& matInvProj = glm::inverse(matProj);
  const float zHalfSize = (zFar - zNear) * 0.5f;
  const float zCenter = -(zNear + zHalfSize);
  frustum->baseFrustum = CreateSubFrustum(matInvProj, 0, 0, screenSizeX, screenSizeY, zNear, zFar);
  frustum->zNear = -zNear;
  frustum->zFar = -zFar;

  static const glm::vec2 tileSize =
    glm::vec2(screenSizeX, screenSizeY) / glm::vec2(tileCountX, tileCountY);
  for (int y = 0; y < tileCountY; ++y) {
    for (int x = 0; x < tileCountX; ++x) {
      const float x0 = static_cast<float>(x) * tileSize.x;
      const float x1 = x0 + tileSize.x;
      const float y0 = static_cast<float>(y) * tileSize.y;
      const float y1 = y0 + tileSize.y;
      frustum->tiles[y][x] = CreateSubFrustum(matInvProj, x0, y0, x1, y1, zNear, zFar);
    }
  }
  return frustum;
}

/**
* ���ƃT�u������̌�������.
*
* @param sphere  ��.
* @param frustum �T�u������.
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
*/
bool SphereInsideSubFrustum(const Sphere& sphere, const SubFrustum& frustum)
{
  const glm::vec3 d = glm::max(glm::vec3(0), glm::abs(frustum.aabbCenter - sphere.center) - frustum.aabbHalfSize);
  if (glm::dot(d, d) > sphere.radius * sphere.radius) {
    return false;
  }
  for (const auto& plane : frustum.planes) {
    if (!SphereInsidePlane(sphere, plane)) {
      return false;
    }
  }
  return true;
}

/**
* ���Ǝ�����̌�������.
*
* @param sphere  ��.
* @param frustum ������.
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
*/
bool SphereInsideFrustum(const Sphere& sphere, const Frustum& frustum)
{
  if (sphere.center.z - sphere.radius > frustum.zNear) {
    return false;
  }
  if (sphere.center.z + sphere.radius < frustum.zFar) {
    return false;
  }
  return SphereInsideSubFrustum(sphere, frustum.baseFrustum);
}

/**
* �~���ƃT�u������̌�������.
*
* @param cone    �~��.
* @param frustum �T�u������.
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
*/
bool ConeInsideSubFrustum(const Cone& cone, const SubFrustum& frustum)
{
  for (const auto& plane : frustum.planes) {
    if (!ConeInsidePlane(cone, plane)) {
      return false;
    }
  }
  return true;
}

/**
* �~���Ǝ�����̌�������.
*
* @param cone    �~��.
* @param frustum ������.
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
*/
bool ConeInsideFrustum(const Cone& cone, const Frustum& frustum)
{
  if (!ConeInsidePlane(cone, Plane{
    glm::vec3(0, 0, frustum.zNear), glm::vec3(0, 0, -1) })) {
    return false;
  }
  if (!ConeInsidePlane(cone, Plane{
    glm::vec3(0, 0, frustum.zFar), glm::vec3(0, 0, 1) })) {
    return false;
  }
  return ConeInsideSubFrustum(cone, frustum.baseFrustum);
}

/**
* �R���X�g���N�^.
*/
LightManager::LightManager()
{
  lights.reserve(1024);
  ssbo[0] = std::make_shared<ShaderStorageBufferObject>(sizeof(LightData));
  ssbo[1] = std::make_shared<ShaderStorageBufferObject>(sizeof(LightData));
}

/**
* ���C�g���쐬����.
*
* @param position ���C�g�̍��W.
* @param color    ���C�g�̖��邳.
*
* @return �쐬�������C�g�ւ̃|�C���^.
*/
LightPtr LightManager::CreateLight(const glm::vec3& position, const glm::vec3& color)
{
  LightPtr p = std::make_shared<Light>();
  p->position = position;
  p->color = color;
  lights.push_back(p);
  return p;
}

/**
* �X�|�b�g���C�g���쐬����.
*
* @param position     ���C�g�̍��W.
* @param color        ���C�g�̖��邳.
* @param direction    ���C�g�̕���.
* @param coneAngle    �X�|�b�g���C�g���Ƃ炷�p�x(�P��=���W�A��).
* @param falloffAngle �X�|�b�g���C�g�̌����J�n�p�x(�P��=���W�A��).
*
* @return �쐬�������C�g�ւ̃|�C���^.
*/
LightPtr LightManager::CreateSpotLight(const glm::vec3& position, const glm::vec3& color,
  const glm::vec3& direction, float coneAngle, float falloffAngle)
{
  LightPtr p = std::make_shared<Light>();
  p->type = Light::Type::SpotLight;
  p->position = position;
  p->color = color;
  p->direction = direction;
  p->coneAngle = coneAngle;
  p->falloffAngle = falloffAngle;
  lights.push_back(p);
  return p;
}

/**
* ���C�g���폜����.
*
* @param light �폜���郉�C�g�ւ̃|�C���^.
*/
void LightManager::RemoveLight(const LightPtr& light)
{
  auto itr = std::find(lights.begin(), lights.end(), light);
  if (itr != lights.end()) {
    lights.erase(itr);
  }
}

/**
* ���C�g���擾����.
*
* @param n ���C�g�̔ԍ�.
*
* @return n�Ԗڂ̃��C�g�ւ̃|�C���^.
*         ���C�g�̐���n�����̏ꍇ��nullptr��Ԃ�.
*/
LightPtr LightManager::GetLight(size_t n) const
{
  if (n >= lights.size()) {
    return nullptr;
  }
  return lights[n];
}

/**
* ���C�g�̐����擾����.
*
* @return ���C�g�̐�.
*/
size_t LightManager::LightCount() const
{
  return lights.size();
}

/**
* GPU�ɓ]�����郉�C�g�f�[�^���X�V����.
*
* @param matView �`��Ɏg�p����r���[�s��.
* @param frustum ������.
*/
void LightManager::Update(const glm::mat4& matView, const FrustumPtr& frustum)
{
  std::shared_ptr<LightData> tileData = std::make_unique<LightData>();

  // �v�Z�ߒ��Ŏg�����ԃf�[�^.
  struct IntermediateData
  {
    enum class Shape {
      sphere,
      cone,
    };
    Shape shape;
    Sphere sphere;
    Cone cone;
  };
  std::vector<IntermediateData> dataView;
  dataView.reserve(lights.size());

  // ���C�g�̌������r���[���W�n�ɕϊ����邽�߂̉�]�s������.
  const glm::mat3 matViewRot = glm::transpose(glm::inverse(glm::mat3(matView)));

  // ���C���t���X�^���ƌ������Ă��郉�C�g���������C�g���X�g�ɓo�^.
  for (glm::uint i = 0; i < lights.size(); ++i) {
    const LightPtr& e = lights[i];

    // ���C�g�̍��W���r���[���W�n�ɕϊ�.
    const glm::vec3 pos = matView * glm::vec4(e->position, 1);

    // ���C�g�̉e���͈͂��v�Z.
    float range = sqrt(
      glm::max(e->color.r, glm::max(e->color.g, e->color.b)));

    // ���C�g�̉e���͈͂ƃ��C���t���X�^���̌���������s��.
    if (e->type == Light::Type::PointLight) {
      // �|�C���g���C�g�̔���.
      range *= 1;
      const Sphere sphere = { pos, range };
      if (SphereInsideFrustum(sphere, *frustum)) {
        // �������Ă���̂Ń��C�g��o�^.
        tileData->lights[dataView.size()] = LightForShader{
          glm::vec4(e->position, static_cast<float>(Light::Type::PointLight)),
          glm::vec4(e->color, range) };

        // ���ԃf�[�^��o�^.
        IntermediateData intermediate;
        intermediate.shape = IntermediateData::Shape::sphere;
        intermediate.sphere = sphere;
        dataView.push_back(intermediate);
        if (dataView.size() >= maxLightCount) {
          break;
        }
      }
    } else if (e->type == Light::Type::SpotLight) {
      // �X�|�b�g���C�g�̔���.
      // 60�x���傫���ꍇ�͋��ő�p.
      range *= 2;
      bool hasIntersect = false;
      if (e->coneAngle > glm::radians(60.0f)) {
        const Sphere sphere = { pos, range };
        if (SphereInsideFrustum(sphere, *frustum)) {
          // �������Ă���̂Ń��C�g��o�^.
          tileData->lights[dataView.size()] = LightForShader{
            glm::vec4(e->position, static_cast<float>(Light::Type::SpotLight)),
            glm::vec4(e->color, range),
            glm::vec4(e->direction, 1),
            glm::vec4(std::cos(e->coneAngle), std::cos(e->falloffAngle), 0, 1) };

          // ���ԃf�[�^��o�^.
          IntermediateData intermediate;
          intermediate.shape = IntermediateData::Shape::sphere;
          intermediate.sphere = sphere;
          dataView.push_back(intermediate);
        }
      } else {
        const glm::vec3 dir = matViewRot * e->direction;
        const float radius = std::sin(e->coneAngle) * range / std::cos(e->coneAngle);
        const Cone cone = { pos, range, dir, radius };
        if (ConeInsideFrustum(cone, *frustum)) {
          // �������Ă���̂Ń��C�g��o�^.
          tileData->lights[dataView.size()] = LightForShader{
            glm::vec4(e->position, static_cast<float>(Light::Type::SpotLight)),
            glm::vec4(e->color, range),
            glm::vec4(e->direction, 1),
            glm::vec4(std::cos(e->coneAngle), std::cos(e->falloffAngle), 0, 1) };

          // ���ԃf�[�^��o�^.
          IntermediateData intermediate;
          intermediate.shape = IntermediateData::Shape::cone;
          intermediate.cone = cone;
          dataView.push_back(intermediate);
        }
      }
      if (dataView.size() >= maxLightCount) {
        break;
      }
    }
  }

  // ���ׂẴT�u�t���X�^���ɂ��āA�o�^���ꂽ���C�g�Ƃ̌���������s��.
  for (int y = 0; y < tileCountY; ++y) {
    for (int x = 0; x < tileCountX; ++x) {
      const SubFrustum& f = frustum->tiles[y][x];
      glm::uint count = 0;
      for (glm::uint i = 0; i < dataView.size(); ++i) {
        const IntermediateData::Shape shape = dataView[i].shape;
        if (shape == IntermediateData::Shape::sphere) {
          if (SphereInsideSubFrustum(dataView[i].sphere, f)) {
            /*
            // ���̒��S���ƃT�u�t���X�^���̍ł��߂�X����̈ʒupz���v�Z.
            float px = a.position.z;
            if (px < b.position.x + b.collision.boxMin.z) {
              px = b.position.z + b.collision.boxMin.z;
            } else if (pz > b.position.z + b.collision.boxMax.z) {
              px = b.position.z + b.collision.boxMax.z;
            }
            // �~���̒��S������ł��߂��_�܂ł̋�����2��(d2)���v�Z.
            const float dx = a.position.x - px;
            const float dz = a.position.z - pz;
            const float d2 = dx * dx + dz * dz;
            // d2���~���̔��a��2��ȏ�Ȃ�Փ˂��Ă��Ȃ�.
            if (d2 >= a.collision.radius * a.collision.radius) {
              return false;
            }
            */
            tileData->lightIndices[y][x][count] = i;
            ++count;
            if (count >= maxLightCountInTile) {
              std::cerr << "[���]" << __func__ <<
                "�T�C�Y�I�[�o�[(�|�C���g���C�g)[" << y << "][" << x << "]\n";
              break;
            }
          }
        } else if (shape == IntermediateData::Shape::cone) {
          if (ConeInsideSubFrustum(dataView[i].cone, f)) {
            tileData->lightIndices[y][x][count] = i;
            ++count;
            if (count >= maxLightCountInTile) {
              std::cerr << "[���]" << __func__ <<
                "�T�C�Y�I�[�o�[(�X�|�b�g���C�g)[" << y << "][" << x << "]\n";
              break;
            }
          }
        }
      }
      tileData->lightCounts[y][x] = count;
    }
  }

  // LightData��GPU�������ɓ]��.
  auto targetSsbo = ssbo[writingSsboNo];
  const size_t size = sizeof(LightData::lightCounts) +
    sizeof(LightData::lightIndices) + sizeof(LightForShader) * dataView.size();
  targetSsbo->CopyData(tileData.get(), size, 0);
  writingSsboNo = !writingSsboNo;
}

/**
* SSBO���O���t�B�b�N�X�p�C�v���C���Ɋ��蓖�Ă�.
*
* @param location ���蓖�Đ�̃��P�[�V�����ԍ�.
*/
void LightManager::Bind(GLuint location) const
{
  ssbo[!writingSsboNo]->Bind(location);
}

/**
* SSBO�̃O���t�B�b�N�X�p�C�v���C���ւ̊��蓖�Ă���������.
*
* @param location ���蓖�Đ�̃��P�[�V�����ԍ�.
*/
void LightManager::Unbind(GLuint location) const
{
  ssbo[!writingSsboNo]->Unbind(location);
}

} // namespace Light

