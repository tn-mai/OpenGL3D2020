/**
* @file MainGameScene.cpp
*
* 時は21世紀。「ナチス2.0」の科学中佐「ゲラルト・ベルコフ」(47)は、人間をゾンビ化する秘術「デス・ストラクチャリング」を実用化。
* ナチス復活と世界征服の野望の手始めに、とあるヨーロッパの辺鄙な片田舎をゾンビまみれにしようとしていた。
*
* SAS予備役でサバイバル・インストラクターの「ジョン・スミス」(31)は、たまたま夏季休暇で訪れた町でこの奇貨に巻き込まれてしまう。
* かろうじて逃げ込んだ町の教会で、牧師の「アンソニー・ウェスト」から驚くべき事実を告げられたのだった。
* 「あなたは神が遣わされた戦士なのです。この「祝福されたAK-47」と「聖なる手榴弾」をお取りなさい。」
* 「そして神の栄光をもって、邪な死者どもに安息をもたらすのです。」
* ジョンは驚きながらも「無辜の市民を守るのが軍人の務め。」という上官「エドガー・ラッセル」(35)の教えを思い出し、運命に従うことにする。
* ナチス2.0の野望をくじくため、荒れ果てた町とゾンビの巣食う深い森を駆け抜けるジョンの過酷な戦いが始まった！
*
* 「遠慮はいらねえ。奴らはとっくに死んでるんだ。」
*
* [後期の内容候補]
*
* 方向性:
* - ホラー感より爽快感を重視.
* - ホラー感はゾンビの大群という時点で十分.
* - 派手な爆発、炎上、誘爆、そして飛び散る血しぶき.
*
* プログラムの強化点:
* - サウンド.
* - マウスによる照準と射撃. 半直線と衝突図形との衝突判定が必要.
* - 球の衝突判定.
* - Y軸回転した直方体の衝突判定.
* - フェードイン・アウト.
* - モーフィングによる補完アニメーション
* - オフスクリーンサーフェスによる画面効果.
* - ノーマルマップ.
* - 三角ポリゴン・四角ポリゴンと直線の衝突判定.
* - モデルデータの軽量化.
* - 専用のモデルフォーマットの開発.
*
* プレイヤーの機能:
* - 手榴弾を投げる. 転がす(追加のアニメーションが必要).
* - 下方向への攻撃手段.
* - バリケード作成.
* - 前転、振りほどき、ジャンプ、吹き飛びダウン、ダウンからの起き上がり.
* - 近接攻撃.
*
* ゾンビの機能:
* - ゾンビモデルのバリエーションを増やす.
* - 下半身損傷or欠損による匍匐移動および攻撃.
* - 吹き飛びダウン.
* - ダウンからの起き上がり.
* - 頭部爆散からのダウン.
* - 上半身爆散からのダウン.
* - 炎上.
* - 炎上ダウン.
* - 噛みつき攻撃.
* - すしざんまいのポーズ.
* - ボスゾンビ.
*/
#define NOMINMAX
#include "MainGameScene.h"
#include "GameData.h"
#include "SceneManager.h"
#include "Actors/PlayerActor.h"
#include "Actors/ZombieActor.h"
#include "Audio.h"
#include "Audio/MainWorkUnit/SE.h"
#include "Audio/MainWorkUnit/BGM.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

/**
* 並木を描画する.
*/
void MainGameScene::AddLineOfTrees(const glm::vec3& start, const glm::vec3& direction)
{
  GameData& global = GameData::Get();

  glm::vec3 offset = start;
  for (float i = 0; i < 19; ++i) {
    std::shared_ptr<Actor> actor = std::make_shared<Actor>("tree",
      &global.primitiveBuffer.Get(GameData::PrimNo::tree),
      texTree, start + direction * i);
    actor->rotation.y = glm::radians(i * 30);
    actor->SetBoxCollision(glm::vec3(-1, 0, -1), glm::vec3(1, 100, 1));
    actors.push_back(actor);
  }
}

/**
* メインゲーム画面を初期化する.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool MainGameScene::Initialize()
{
  // FBOを初期化する.
  int w, h;
  glfwGetFramebufferSize(GameData::Get().window, &w, &h);
  fbo = std::make_shared<FramebufferObject>(w, h, FboType::ColorDepthStencil);
  if (!fbo || !fbo->GetId()) {
    return false;
  }

  // デプシャドウマッピング用のFBOを作成.
  fboShadow = std::make_shared<FramebufferObject>(4096, 4096, FboType::Depth);
  if (!fboShadow || !fboShadow->GetId()) {
    return false;
  }

  // ブルームエフェクト用のFBOを作成.
  int bloomW = w;
  int bloomH = h;
  for (size_t i = 0; i < std::size(fboBloom); ++i) {
    bloomW /= 2;
    bloomH /= 2;
    fboBloom[i]= std::make_shared<FramebufferObject>(bloomW, bloomH, FboType::Color);
    if (!fboBloom[i]|| !fboBloom[i]->GetId()) {
      return false;
    }
  }

  texGround = std::make_shared<Texture::Image2D>("Res/Ground.tga");
  texTree   = std::make_shared<Texture::Image2D>("Res/Tree.tga");
  texHouse  = std::make_shared<Texture::Image2D>("Res/House.tga");
  texCube   = std::make_shared<Texture::Image2D>("Res/Rock.tga");
  texZombie = std::make_shared<Texture::Image2D>("Res/zombie_male.tga");
  texPlayer = std::make_shared<Texture::Image2D>("Res/player_male.tga");
  texGameClear = std::make_shared<Texture::Image2D>("Res/Survived.tga");
  texGameOver = std::make_shared<Texture::Image2D>("Res/GameOver.tga");
  texBlack = std::make_shared<Texture::Image2D>("Res/Black.tga");
  texPointer = std::make_shared<Texture::Image2D>("Res/Pointer.tga");
  texStreetLampA = std::make_shared<Texture::Image2D>("Res/street_lamp_a.tga");
  texStreetLampBC = std::make_shared<Texture::Image2D>("Res/street_lamp_bc.tga");
  texStreetLampD = std::make_shared<Texture::Image2D>("Res/street_lamp_d.tga");

  GameData& global = GameData::Get();

  // 地面を表示.
  {
    std::shared_ptr<Actor> actor = std::make_shared<Actor>(
      "ground", &global.primitiveBuffer.Get(GameData::PrimNo::ground), texGround, glm::vec3(0, 0, 0));
    actor->samplers[0] = std::make_shared<Texture::Sampler>();
    actor->samplers[0]->SetFilter(GL_LINEAR);
    actor->samplers[0]->SetWrapMode(GL_REPEAT);
    actor->samplers[1] = actor->samplers[0];
    actor->samplers[2] = actor->samplers[0];
    actor->texNormal = std::make_shared<Texture::Image2D>("Res/Ground_normal.tga", Texture::ImageType::non_color);
    actor->texMetallicSmoothness =
      std::make_shared<Texture::Image2D>("Res/Ground_spec.tga", Texture::ImageType::non_color);
    actor->SetBoxCollision(glm::vec3(-20, -10, -20), glm::vec3(20, 0, 20));
    actor->isShadowCaster = false;
    actors.push_back(actor);
  }

  // 木を表示.
  for (float j = 0; j < 4; ++j) {
    const glm::mat4 matRot = glm::rotate(glm::mat4(1), glm::radians(90.0f) * j, glm::vec3(0, 1, 0));
    AddLineOfTrees(matRot * glm::vec4(-19, 0, 19, 1), matRot * glm::vec4(2, 0, 0, 1));
  }

  // 家を表示.
  {
    std::shared_ptr<Actor> actor = std::make_shared<Actor>(
      "house", &global.primitiveBuffer.Get(GameData::PrimNo::house), texHouse, glm::vec3(0));
    actor->samplers[0] = std::make_shared<Texture::Sampler>();
    actor->samplers[0]->SetFilter(GL_NEAREST);
    actor->samplers[0]->SetWrapMode(GL_CLAMP_TO_EDGE);
    actor->samplers[2] = actor->samplers[0];
    //actor->texNormal = std::make_shared<Texture::Image2D>("Res/house_normal.tga");
    //actor->texMetallicSmoothness = std::make_shared<Texture::Image2D>("Res/House_spec.tga", false);
    actor->SetBoxCollision(glm::vec3(-3, 0, -3), glm::vec3(3, 5, 3));
    actors.push_back(actor);
  }

  // 立方体を表示.
  {
    std::shared_ptr<Actor> actor = std::make_shared<Actor>(
      "cube", &global.primitiveBuffer.Get(GameData::PrimNo::cube), texCube, glm::vec3(10, 1, 0));
    actor->SetBoxCollision(glm::vec3(-1), glm::vec3(1));
    actors.push_back(actor);
  }

  // プレイヤーを表示.
  {
    playerActor = std::make_shared<PlayerActor>(glm::vec3(10, 0, 10), 0.0f, this);
    actors.push_back(playerActor);
  }

  // ゾンビを表示.
  const Mesh::Primitive* pPrimitive = &global.primitiveBuffer.Get(GameData::PrimNo::zombie_male_walk_0);
  for (size_t i = 0; i < appearanceEnemyCount; ++i) {
    glm::vec3 pos(0);
    pos.x = std::uniform_real_distribution<float>(-18, 18)(global.random);
    pos.z = std::uniform_real_distribution<float>(-18, 18)(global.random);
    float rotY =
      std::uniform_real_distribution<float>(0, glm::radians(360.0f))(global.random);
    std::shared_ptr<Actor> actor = std::make_shared<ZombieActor>(pos, rotY, this);
    actors.push_back(actor);
  }

  // 点光源を設定する
  pointLight = Shader::PointLight{
    glm::vec4(8, 10,-8, 0),
    glm::vec4(0.4f, 0.7f, 1.0f, 0) * 200.0f
  };

  lightManager = std::make_shared<Light::LightManager>();

  // ライトフラスタムを作成.
  {
    GameData& gamedata = GameData::Get();
    int w, h;
    glfwGetWindowSize(GameData::Get().window, &w, &h);
    const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    matProj = glm::perspective(glm::radians(45.0f), aspectRatio, 1.0f, 500.0f);
    frustum = Light::CreateFrustum(matProj, 1, 500);

    for (int i = 0; i < 100; ++i) {
      //const float x = std::uniform_real_distribution<float>(-18, 18)(gamedata.random);
      //const float y = std::uniform_real_distribution<float>(0.5f, 3)(gamedata.random);
      //const float z = std::uniform_real_distribution<float>(-18, 18)(gamedata.random);
      const float x = static_cast<float>(i % 10) * 4 - 18;
      const float z = static_cast<float>(i / 10) * 4 - 18;
      const int c = std::uniform_int_distribution<>(1, 7)(gamedata.random);
      const float range = std::uniform_real_distribution<float>(0.5f, 1.5f)(gamedata.random);

      const glm::vec3 color = glm::vec3(c & 1, (c >> 1) & 1, (c >> 2) & 1) * range * range * 3.14f;
      lightManager->CreateLight(glm::vec3(x, 1.0f, z), color);
    }
    flashLight = lightManager->CreateSpotLight(playerActor->position + glm::vec3(0, 1, 0),
      glm::vec3(1, 0.9f, 0.8f) * 30.0f, glm::vec3(1, 0, 0),
      glm::radians(20.0f), glm::radians(5.0f));
  }

  // 街灯を表示
  if (1) {
    static const glm::vec3 pos[] = {
      { -16, 0, -16 }, { -16, 0, -10 }, { -16, 0, -4 },
      { -16, 0,   2 }, { -16, 0,   8 }, { -16, 0, 14 },
      { -6,  0,   6 }, {  6, 0, 6},
    };
    for (const auto& e : pos) {
      std::shared_ptr<Actor> actor = std::make_shared<Actor>(
        "StreetLamp", &global.primitiveBuffer.Get(GameData::PrimNo::street_lamp_a), texStreetLampA, e);
      actor->SetBoxCollision(glm::vec3(-0.2f, 0, -0.2f), glm::vec3(0.2f, 4, 0.2f));
      //actor->rotation.y = glm::radians(180.0f);
      actors.push_back(actor);
      if (e.x == 6) {
        actor->rotation.y = 3.1415f;
        lightManager->CreateSpotLight(e + glm::vec3(-1, 4, 0), glm::vec3(1, 0.9f, 0.8f) * 100.0f, glm::vec3(0, -1, 0),
          glm::radians(30.0f), glm::radians(15.0f));
      } else {
        lightManager->CreateSpotLight(e + glm::vec3(1, 4, 0), glm::vec3(1, 0.9f, 0.8f) * 100.0f, glm::vec3(0, -1, 0),
          glm::radians(30.0f), glm::radians(15.0f));
      }
    }
  }

  // ゲームデータの初期設定.
  GameData& gamedata = GameData::Get();
  gamedata.killCount = 0;

  // マウスカーソルを非表示にする.
  glfwSetInputMode(gamedata.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

  // 新規アクター追加用の配列のサイズを予約する.
  newActors.reserve(100);

  sprites.reserve(1000);
  spriteRenderer.Allocate(1000);

  Audio::Instance().Play(4, CRI_BGM_BENSOUND_HIGHOCTANE);

  std::cout << "[情報] MainGameSceneを開始.\n";
  return true;
}

/**
* メインゲーム画面のキー入力を処理する.
*
* @param window    GLFWウィンドウへのポインタ.
*/
void MainGameScene::ProcessInput(GLFWwindow* window)
{
  // クリアしている?
  GameData& gamedata = GameData::Get();
  if (isGameClear) {
    // Enterキーが押されたらタイトル画面に移動.
    if (gamedata.keyPressedInLastFrame & GameData::Key::enter) {
      SceneManager::Get().ChangeScene(TITLE_SCENE_NAME);
    }
    return;
  }

  // ゲームオーバー?
  if (isGameOver) {
    // Enterキーが押されたらタイトル画面に移動.
    if (gamedata.keyPressedInLastFrame & GameData::Key::enter) {
      SceneManager::Get().ChangeScene(TITLE_SCENE_NAME);
    }
    return;
  }

  // プレイヤーが死んでいたら
  if (!isGameOver) {
    if (playerActor->state == Actor::State::dead) {
      if (flashLight) {
        lightManager->RemoveLight(flashLight);
        flashLight.reset();
      }
      // アニメーションが終了していたらゲームオーバーにする.
      if (playerActor->animationNo >= playerActor->animation->list.size() - 1) {
        Audio::Instance().Stop(4);
        Audio::Instance().Play(4, CRI_BGM_GAME_OVER);
        isGameOver = true;
      }
      return;
    }
  }

  // マウスポインタを地面に表示.
  {
    const glm::vec2 cursor = GameData::Get().cursorPosition;

    Segment seg;
    const glm::vec2 screenPosition((cursor.x / 1280) * 2, (cursor.y / 720) * 2);
    const glm::mat4 matInverseVP = glm::inverse(matProj * matView);

    const glm::vec4 start = matInverseVP * glm::vec4(screenPosition, -1, 1);
    seg.start = glm::vec3(start) / start.w;

    const glm::vec4 end = matInverseVP * glm::vec4(screenPosition, 1, 1);
    seg.end = glm::vec3(end) / end.w;

    const Plane plane{ playerActor->position + glm::vec3(0, 1, 0), glm::vec3(0, 1, 0) };
    Intersect(seg, plane, &posMouseCursor);

    const glm::vec3 direction(posMouseCursor - playerActor->position);
    playerActor->rotation.y = std::atan2(-direction.z, direction.x);
  }

  playerActor->ProcessInput();
  if (!newActors.empty()) {
    actors.insert(actors.end(), newActors.begin(), newActors.end());
    newActors.clear();
  }
}

/**
* メインゲーム画面を更新する.
*
* @param window    GLFWウィンドウへのポインタ.
* @param deltaTime 前回の更新からの経過時間.
*/
void MainGameScene::Update(GLFWwindow* window, float deltaTime)
{
  const float maxDeltaTime = 1.0f / 30.0f;
  remainingDeltaTime += deltaTime;
  deltaTime = std::min(remainingDeltaTime, maxDeltaTime);
  do {
    // アクターリストに含まれるアクターの状態を更新する.
    UpdateActorList(actors, deltaTime);

    // 新規アクターが存在するなら、それをアクターリストに追加する.
    if (!newActors.empty()) {
      actors.insert(actors.end(), newActors.begin(), newActors.end());
      newActors.clear();
    }

    // 衝突判定.

    // 衝突する可能性のあるペアを格納する配列.
    std::vector<std::pair<Actor&, Actor&>> potentiallyCollidingPairs;
    potentiallyCollidingPairs.reserve(1000);

    // 衝突したペアを格納する配列.
    std::vector<std::pair<Actor*, Actor*>> collidedPairs;
    collidedPairs.reserve(1000);

    // 衝突判定を持つアクターだけを取り出す.
    ActorList actorsWithCollider;
    actorsWithCollider.reserve(actors.size());
    std::copy_if(
      actors.begin(), actors.end(), std::back_inserter(actorsWithCollider),
      [](const ActorPtr& a) { return a->collision.shape != Collision::Shape::none; });

    // 衝突しそうなペアをリストアップ.
    for (size_t ia = 0; ia < actorsWithCollider.size(); ++ia) {
      Actor& a = *actorsWithCollider[ia]; // アクターA
      // 計算済み及び自分自身を除く、残りのアクターとの間で衝突判定を実行.
      for (size_t ib = ia + 1; ib < actorsWithCollider.size(); ++ib) {
        Actor& b = *actorsWithCollider[ib]; // アクターB

        // 2つの衝突AABBが交差していなければ衝突しない(多分).
        const glm::vec3 d = glm::abs(b.boundingBox.c - a.boundingBox.c);
        const glm::vec3 r = a.boundingBox.r + b.boundingBox.r;
        if (d.x > r.x || d.y > r.y || d.z > r.z) {
          continue;
        }

        // どちらかがブロックしない、または衝突形状が直方体同士の場合はここで衝突判定を行う.
        if (!a.collision.blockOtherActors || !b.collision.blockOtherActors || (
          a.collision.shape == Collision::Shape::box &&
          b.collision.shape == Collision::Shape::box)) {
          if (DetectCollision(a, b, true)) {
            collidedPairs.emplace_back(&a, &b);
          }
          continue;
        }

        // ここまで来たら衝突の可能性がある.
        potentiallyCollidingPairs.emplace_back(a, b);
      }
    }

    // 本当に衝突するかを調べる.
    for (size_t loop = 0; loop < 5; ++loop) {
      bool hasCollision = false;
      for (auto& pair : potentiallyCollidingPairs) {
          if (DetectCollision(pair.first, pair.second, true)) {
            collidedPairs.emplace_back(&pair.first, &pair.second);
            hasCollision = true;
          }
      }
      if (!hasCollision) {
        break;
      }
    }

    // 重複するペアを除外する.
    std::sort(collidedPairs.begin(), collidedPairs.end());
    const auto itr = std::unique(collidedPairs.begin(), collidedPairs.end());
    collidedPairs.erase(itr, collidedPairs.end());

    // OnHitを呼び出す.
    for (auto& e : collidedPairs) {
      e.first->OnHit(*e.first, *e.second);
      e.second->OnHit(*e.second, *e.first);
    }

    // 今回の更新で消費した時間を残り経過時間から減算.
    // 経過時間が固定値以上ならループ.
    remainingDeltaTime -= deltaTime;
  } while (remainingDeltaTime >= maxDeltaTime);

  // まだクリアしていない?
  if (!isGameClear) {
    // クリア条件(「倒した敵の数」が「出現する敵の数」以上)を満たしている?
    if (GameData::Get().killCount >= appearanceEnemyCount) {
      // ゲームクリアフラグをtrueにする.
      isGameClear = true;

      // プレイヤーアクターを待機状態にする.
      playerActor->velocity = glm::vec3(0);
      playerActor->SetAnimation(GameData::Get().anmPlayerIdle);

      Audio::Instance().Stop(4);
      Audio::Instance().Play(4, CRI_BGM_SUCCESS);

      std::cout << "[情報] ゲームクリア条件を達成\n";
    }
  }

  // カメラをプレイヤーアクターのななめ上に配置.
  const glm::vec3 viewPosition = playerActor->position + glm::vec3(0, 7, 7);
  // プレイヤーアクターの足元が画面の中央に映るようにする.
  const glm::vec3 viewTarget = playerActor->position;

  // 座標変換行列を作成.
  int w, h;
  glfwGetWindowSize(window, &w, &h);
  const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
  matProj =
    glm::perspective(glm::radians(45.0f), aspectRatio, 1.0f, 500.0f);
  matView =
    glm::lookAt(viewPosition, viewTarget, glm::vec3(0, 1, 0));

  UpdateSpriteList(sprites, deltaTime);
  spriteRenderer.Update(sprites, matView);

  if (flashLight) {
    flashLight->position = playerActor->position + glm::vec3(0, 1.2f, 0);
    flashLight->direction = glm::rotate(glm::mat4(1),
      playerActor->rotation.y, glm::vec3(0, 1, 0)) * glm::vec4(1, -0.1f, 0, 1);
    flashLight->direction = glm::normalize(flashLight->direction);
  }
  lightManager->Update(matView, frustum);
}

/**
* メインゲーム画面を描画する.
*
* @param window    GLFWウィンドウへのポインタ.
*/
void MainGameScene::Render(GLFWwindow* window) const
{
  int fbw, fbh;
  glfwGetFramebufferSize(window, &fbw, &fbh);
  if (fbw <= 0 || fbh <= 0) {
    return;
  }

  GameData& global = GameData::Get();
  GameData& gamedata = GameData::Get();
  std::shared_ptr<Shader::Pipeline> pipeline = global.pipeline3D;
  Mesh::PrimitiveBuffer& primitiveBuffer = global.primitiveBuffer;

#define TIME_SETTING_NOON 0
#define TIME_SETTING_SUNSET 1
#define TIME_SETTING_NIGHT 2

#define TIME_SETTING TIME_SETTING_NOON

  const Shader::DirectionalLight directionalLight{
    glm::normalize(glm::vec4(3,-2,-2, 0)),
#if TIME_SETTING == TIME_SETTING_NOON
    glm::vec4(glm::vec3(1, 0.9f, 0.8f) * 4.0f, 1) // 昼
#elif TIME_SETTING == TIME_SETTING_SUNSET
    glm::vec4(glm::vec3(1, 0.5f, 0.2f) * 2.0f, 1) // 夕方
#else // TIME_SETTING == TIME_SETTING_NIGHT
    glm::vec4(glm::vec3(1, 0.9f, 0.8f) * 0.25f, 1) // 夜
#endif
  };

  // 影描画用FBOに描画する.
  {
    // 描画先を影描画用FBOに変更.
    fboShadow->Bind();
    glDisable(GL_FRAMEBUFFER_SRGB); // ガンマ補正を無効にする.
    glDisable(GL_BLEND); // アルファブレンドを無効にする.
    glEnable(GL_DEPTH_TEST); // 深度テストを有効にする.
    glEnable(GL_CULL_FACE); // 裏面カリングを有効にする.

    // 深度バッファをクリア.
    glClear(GL_DEPTH_BUFFER_BIT);

    primitiveBuffer.BindVertexArray();
    std::shared_ptr<Shader::Pipeline> pipelineShadow = gamedata.pipelineShadow;
    pipelineShadow->Bind();
    gamedata.sampler.Bind(0);

    // 座標変換行列を作成.
    const float halfW = 50;
    const float halfH = 50;
    glm::mat4 matProj =
      glm::ortho(-halfW, halfW, -halfH, halfH, 20.0f, 200.0f);
      //glm::perspective(glm::radians(30.0f), 1.0f, 20.0f, 200.0f);

    const glm::vec3 viewPosition = playerActor->position - glm::vec3(directionalLight.direction) * 100.0f;
    //const glm::vec3 viewPosition = -glm::vec3(directionalLight.direction) * 50.0f;
    const glm::vec3 viewTarget = playerActor->position;
    glm::mat4 matView =
      glm::lookAt(viewPosition, viewTarget, glm::vec3(0, 1, 0));

    glm::mat4 matShadowVP = matProj * matView;
    for (size_t i = 0; i < actors.size(); ++i) {
      actors[i]->Draw(*pipelineShadow, matShadowVP, Actor::DrawType::shadow);
    }

    // クリップ座標系からスクリーン座標系への座標変換行列を追加.
    const glm::mat4 matBiasVP = glm::mat4(
      0.5f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.5f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.5f, 0.0f,
      0.5f, 0.5f, 0.5f, 1.0f
    ) * matShadowVP;
    pipeline->SetShadowMatrix(matBiasVP);
    gamedata.pipelineDeathEffect->SetShadowMatrix(matBiasVP);
  }

  // 描画先をフレームバッファオブジェクトに変更.
  fbo->Bind();
  glDisable(GL_FRAMEBUFFER_SRGB); // ガンマ補正を無効にする

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // 環境光を設定する.
#if TIME_SETTING == TIME_SETTING_NOON
  pipeline->SetAmbientLight(glm::vec3(0.1f, 0.125f, 0.15f)); // 昼
#elif TIME_SETTING == TIME_SETTING_SUNSET
  pipeline->SetAmbientLight(glm::vec3(0.09f, 0.05f, 0.1f)); // 夕方 
#else // TIME_SETTING == TIME_SETTING_NIGHT
  pipeline->SetAmbientLight(glm::vec3(0.02f, 0.01f, 0.03f)); // 夜
#endif

  // 平行光源を設定する
  pipeline->SetLight(directionalLight);

  //pipeline->SetLight(pointLight);
  // ライト用SSBOをバインド.
  lightManager->Bind(0);

  primitiveBuffer.BindVertexArray();
  pipeline->Bind();
  global.sampler.Bind(0);
  global.sampler.Bind(1);
  global.samplerClampToEdge.Bind(2);
  global.samplerClampToEdge.Bind(3);
  global.samplerShadow.Bind(4);

  // アクターリストを描画.
  pipeline->SetViewPosition(playerActor->position + glm::vec3(0, 7, 7));

  std::shared_ptr<Shader::Pipeline> pipelineDeathEffect = global.pipelineDeathEffect;
  pipelineDeathEffect->SetAmbientLight(glm::vec3(0.1f, 0.125f, 0.15f));
  pipelineDeathEffect->SetLight(directionalLight);
  pipelineDeathEffect->SetViewPosition(playerActor->position + glm::vec3(0, 7, 7));

  // シャドウテクスチャをバインド.
  fboShadow->BindDepthStencilTexture(4);

  const glm::mat4 matVP = matProj * matView;
  for (size_t i = 0; i < actors.size(); ++i) {
    actors[i]->Draw(matVP, Actor::DrawType::color);
  }

  global.samplerShadow.Unbind();
  global.samplerClampToEdge.Bind(2);
  global.samplerClampToEdge.Bind(3);

  if (0) {
    // Y軸回転.
    const float degree = static_cast<float>(std::fmod(glfwGetTime() * 180.0, 360.0));
    const glm::mat4 matModelR =
      glm::rotate(glm::mat4(1), glm::radians(degree), glm::vec3(0, 1, 0));
    // 拡大縮小.
    const glm::mat4 matModelS =
      glm::scale(glm::mat4(1), glm::vec3(0.5f, 0.25f, 0.5f));
    // 平行移動.
    const glm::mat4 matModelT =
      glm::translate(glm::mat4(1), glm::vec3(pointLight.position) + glm::vec3(0, -1.25f, 0));
    // 拡大縮小・回転・平行移動を合成.
    const glm::mat4 matModel = matModelT * matModelR * matModelS;
    pipeline->SetMVP(matProj * matView * matModel);
    pipeline->SetModelMatrix(matModel);
    texTree->Bind(0);
    primitiveBuffer.Get(GameData::PrimNo::tree).Draw();
  }

  // アクターの影を描画.
  if (0) {
    // ステンシルバッファを有効にする.
    glEnable(GL_STENCIL_TEST);
    // 「比較に使う値」を1にして、常に比較が成功するように設定.
    glStencilFunc(GL_ALWAYS, 1, 0xff);
    // ステンシル深度の両方のテストに成功した場合に「比較する値」を書き込むように設定.
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    // ステンシルバッファの全ビットの書き込みを許可.
    glStencilMask(0xff);
    // カラーバッファへの書き込みを禁止.
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    // 深度バッファへの書き込みを禁止.
    glDepthMask(GL_FALSE);

    // 高さ1mの物体が落とす影の長さを計算.
    const float scale = 1.0f / -directionalLight.direction.y;
    const float sx = directionalLight.direction.x * scale;
    const float sz = directionalLight.direction.z * scale;

    // ぺちゃんこ行列(Y座標を0にする行列)を作成.
    const glm::mat4 matShadow(
      1.00f, 0.00f, 0.00f, 0.00f,
      sx, 0.00f, sz, 0.00f,
      0.00f, 0.00f, 1.00f, 0.00f,
      0.00f, 0.01f, 0.00f, 1.00f);

    // 影用パイプランをバインド.
    std::shared_ptr<Shader::Pipeline> pipelineShadow = GameData::Get().pipelineShadow;
    pipelineShadow->Bind();

    // ぺちゃんこ行列→ビュー行列→プロジェクション行列の順番に掛ける行列を作る.
    const glm::mat4 matShadowVP = matVP * matShadow;

    // ぺちゃんこビュープロジェクション行列を使って全てのアクターを描画する.
    for (const auto& actor : actors) {
      actor->Draw(*pipelineShadow, matShadowVP, Actor::DrawType::shadow);
    }

    // ステンシル値が1の場合のみテストに成功するように設定.
    glStencilFunc(GL_EQUAL, 1, 0xff);
    // ステンシルバッファ
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    // カラーバッファへの描き込みを許可.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    // 深度バッファを無効化.
    glDisable(GL_DEPTH_TEST);

    // 画面全体に影色を塗る.
    texBlack->Bind(0);
    pipelineShadow->SetMVP(glm::scale(glm::mat4(1), glm::vec3(2)));
    pipelineShadow->SetMorphWeight(glm::vec3(0));
    primitiveBuffer.Get(GameData::PrimNo::plane).Draw();

    // ステンシルバッファを無効化.
    glDisable(GL_STENCIL_TEST);
    // 深度バッファを有効化.
    glEnable(GL_DEPTH_TEST);
    // 深度バッファへの描き込みを許可.
    glDepthMask(GL_TRUE);
  }

  // スプライトを描画.
  spriteRenderer.Draw(pipeline, matProj * matView);

  // ライト用SSBOのバインドを解除.
  lightManager->Unbind(0);

  // 3Dモデル用のVAOをバインドしなおしておく.
  primitiveBuffer.BindVertexArray();

  // ブルームエフェクトを描画.
  {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    gamedata.samplerPostEffect.Bind(0);

    // planeモデルのサイズは0.5x0.5なので1.0x1.0になるような行列を設定.
    const glm::mat4 matMVP = glm::scale(glm::mat4(1), glm::vec3(2));

    // 明るい成分をfboBloom[0]に抽出する.
    gamedata.pipelineHighBrightness->Bind();
    gamedata.pipelineHighBrightness->SetMVP(matMVP);
    fboBloom[0]->Bind();
    fbo->BindColorTexture(0);
    primitiveBuffer.Get(GameData::PrimNo::plane).Draw();

    // 明るい成分をぼかしながら縮小コピー.
    gamedata.pipelineBlur->Bind();
    gamedata.pipelineBlur->SetMVP(matMVP);
    for (size_t i = 1; i < std::size(fboBloom); ++i) {
      fboBloom[i]->Bind();
      fboBloom[i - 1]->BindColorTexture(0);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }

    // ぼかした結果を拡大しながら合計する.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    gamedata.pipelineSimple->Bind();
    gamedata.pipelineSimple->SetMVP(matMVP);
    for (size_t i = std::size(fboBloom) - 1; i > 0; --i) {
      fboBloom[i - 1]->Bind();
      fboBloom[i]->BindColorTexture(0);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }

    // OpenGL機能の設定を元に戻す.
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  // 描画先をデフォルトのフレームバッファに戻す.
  fbo->Unbind();

  glEnable(GL_FRAMEBUFFER_SRGB); // ガンマ補正を有効にする

  // 2D表示.
  {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    // 座標変換行列を作成.
    const float halfW = fbw / 2.0f;
    const float halfH = fbh / 2.0f;
    const glm::mat4 matProj =
      glm::ortho<float>(-halfW, halfW, -halfH, halfH, 1.0f, 500.0f);
    const glm::mat4 matView =
      glm::lookAt(glm::vec3(0, 0, 100), glm::vec3(0), glm::vec3(0, 1, 0));
    const glm::mat4 matVP = matProj * matView;

    std::shared_ptr<Shader::Pipeline> pipeline2D = GameData::Get().pipelineSimple;
    pipeline2D->Bind();

    // 3D描画結果を描画.
    {
      //auto pipeline = GameData::Get().pipelineSobelFilter;
      auto pipeline = GameData::Get().pipelineHatching;
      pipeline->Bind();

      glDisable(GL_BLEND);
      fbo->BindColorTexture(0);
      fbo->BindDepthStencilTexture(2);
      GameData::Get().texHatching->Bind(1);
      fboBloom[0]->BindColorTexture(3);

      const glm::mat4 matModelS = glm::scale(glm::mat4(1),
        glm::vec3(fbw, fbh, 1));
      const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
      const glm::mat4 matModel = matModelT * matModelS;
      pipeline->SetMVP(matVP * matModel);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();

      fboBloom[0]->UnbindColorTexture();
      GameData::Get().texHatching->Unbind();
      fbo->UnbindColorTexture();
      fbo->UnbindDepthStencilTexture();
      glEnable(GL_BLEND);
    }

    pipeline2D->Bind();

    // マウスカーソルを表示.
    {
      // マウスカーソル画像のテクスチャのピクセル数を拡大率に設定.
      const glm::mat4 matModelS = glm::scale(glm::mat4(1),
        glm::vec3(texPointer->Width(), texPointer->Height(), 1));

      // マウスカーソル座標を表示位置に設定.
      const glm::mat4 matModelT = glm::translate(glm::mat4(1),
        glm::vec3(GameData::Get().cursorPosition, 0));

      // MVP行列を計算し、GPUメモリに転送.
      const glm::mat4 matModel = matModelT * matModelS;
      pipeline2D->SetMVP(matVP * matModel);

      // マウスカーソル画像のテクスチャをグラフィックスパイプラインに割り当てる.
      texPointer->Bind(0);

      // 上の設定が適用された四角形を描画.
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }

    // シャドウテクスチャの確認用.
    if (false) {
      const glm::mat4 matModelS = glm::scale(glm::mat4(1),
        glm::vec3(512, 512, 1));
      const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(-200, 100, 0));
      const glm::mat4 matModel = matModelT * matModelS;
      pipeline2D->SetMVP(matVP * matModel);
      fboShadow->BindDepthStencilTexture(0);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }

    if (false) {
      gamedata.samplerNearest.Bind(0);
      glDisable(GL_BLEND);
      glm::vec3 pos(-640, 360, 0);
      for (size_t i = 0; i < std::size(fboBloom); ++i) {
        float w = static_cast<float>(fboBloom[i]->GetWidht());
        float h = static_cast<float>(fboBloom[i]->GetHeight());
        const glm::mat4 matModelS = glm::scale(glm::mat4(1),
          glm::vec3(w, h, 1));
        pos.x = -640 + w / 2;
        pos.y -= h / 2;
        const glm::mat4 matModelT = glm::translate(glm::mat4(1), pos);
        const glm::mat4 matModel = matModelT * matModelS;
        pipeline2D->SetMVP(matVP * matModel);
        fboBloom[i]->BindColorTexture(0);
        primitiveBuffer.Get(GameData::PrimNo::plane).Draw();

        pos.y -= h - h / 2;
      }
      glEnable(GL_BLEND);
      gamedata.samplerClampToEdge.Bind(0);
    }

    // ゲームクリア画像を描画.
    if (isGameClear) {
      const glm::mat4 matModelS = glm::scale(glm::mat4(1),
        glm::vec3(texGameClear->Width() * 2.0f, texGameClear->Height() * 2.0f, 1));
      const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(0, 100, 0));
      const glm::mat4 matModel = matModelT * matModelS;
      pipeline2D->SetMVP(matVP * matModel);
      texGameClear->Bind(0);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }

    // ゲームオーバー画像を描画.
    if (isGameOver) {
      const glm::mat4 matModelS = glm::scale(glm::mat4(1),
        glm::vec3(texGameOver->Width() * 2.0f, texGameOver->Height() * 2.0f, 1));
      const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(0, 100, 0));
      const glm::mat4 matModel = matModelT * matModelS;
      pipeline2D->SetMVP(matVP * matModel);
      texGameOver->Bind(0);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }
  }

  Texture::UnbindAllTextures();
  Texture::UnbindAllSamplers();
  Shader::UnbindPipeline();
  primitiveBuffer.UnbindVertexArray();
}

/**
* メインゲーム画面を終了する.
*/
void MainGameScene::Finalize()
{
  // マウスカーソルを表示する.
  glfwSetInputMode(GameData::Get().window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  Audio::Instance().Stop(4);

  std::cout << "[情報] MainGameSceneを終了.\n";
}

/**
* 新規アクターを追加する.
* 
* @param p 追加するアクターのポインタ.
*/
void MainGameScene::AddActor(ActorPtr p)
{
  newActors.push_back(p);
}

/**
* プレイヤーアクターを取得する.
*
* @return プレイヤーアクターへのポインタ.
*/
ActorPtr MainGameScene::GetPlayerActor()
{
  return playerActor;
}

/**
* マウスカーソル座標を取得する.
*
* @return マウスカーソル座標.
*/
const glm::vec3& MainGameScene::GetMouseCursor() const
{
  return posMouseCursor;
}

/**
* 血しぶきスプライトを追加する.
*
* @param position スプライトを表示する座標.
*/
void MainGameScene::AddBloodSprite(const glm::vec3& position)
{
  GameData& gamedata = GameData::Get();
#if 0
  std::shared_ptr<Sprite> blood =
    std::make_shared<Sprite>(position + glm::vec3(0, 1, 0),gamedata.texBlood);
  const float vx = std::uniform_real_distribution<float>(-3, 3)(gamedata.random);
  const float vy = std::uniform_real_distribution<float>(-3, 3)(gamedata.random);
  const float vz = std::uniform_real_distribution<float>(-3, 3)(gamedata.random);
  blood->velocity = glm::vec3(vx, vy, vz);
  blood->scale = glm::vec2(
    std::uniform_real_distribution<float>(0.25f, 1)(gamedata.random));
  blood->gravityScale = 1;
  blood->lifespan = 0.5f;

  sprites.push_back(blood);
#elif 1
  std::shared_ptr<Sprite> blood =
    std::make_shared<Sprite>(position + glm::vec3(0, 1, 0),gamedata.texBlood);
  const float speed = std::uniform_real_distribution<float>(1.5f, 6.0f)(gamedata.random);
  const glm::mat4 matRotX = glm::rotate(glm::mat4(1),
    std::uniform_real_distribution<float>(0, glm::radians(90.0f))(gamedata.random),
    glm::vec3(1, 0, 0));
  const glm::mat4 matRotY = glm::rotate(glm::mat4(1),
    std::uniform_real_distribution<float>(0, glm::radians(360.0f))(gamedata.random),
    glm::vec3(0, 1, 0));
  blood->velocity = matRotY * matRotX * glm::vec4(0, speed, 0, 1);
  blood->rotation = std::uniform_real_distribution<float>(0, glm::radians(360.0f))(gamedata.random);
  blood->scale = glm::vec3(std::normal_distribution<float>(0, 1)(gamedata.random) * 0.25f + 0.5f);
  blood->angularVelocity= std::uniform_real_distribution<float>(glm::radians(30.0f), glm::radians(180.0f))(gamedata.random);
  blood->lifespan = std::normal_distribution<float>(0, 1)(gamedata.random) * 0.125f + 0.25f;
  blood->color = glm::vec4(1, 1, 1, 2);
  blood->colorVelocity = glm::vec4(0, 0, 0, -4.0f * (blood->lifespan / 0.5f));
  blood->scaleVelocity = glm::vec2(-0.5f);
  blood->gravityScale = 1;
  sprites.push_back(blood);
#else
  ActorPtr blood = std::make_shared<Actor>("blood",
    &gamedata.primitiveBuffer.Get(GameData::PrimNo::plane),
    gamedata.texBlood, position + glm::vec3(0, 1, 0));

  const float speed = std::uniform_real_distribution<float>(1.5f, 6.0f)(gamedata.random);
  const glm::mat4 matRotX = glm::rotate(glm::mat4(1),
    std::uniform_real_distribution<float>(0, glm::radians(90.0f))(gamedata.random),
    glm::vec3(1, 0, 0));
  const glm::mat4 matRotY = glm::rotate(glm::mat4(1),
    std::uniform_real_distribution<float>(0, glm::radians(360.0f))(gamedata.random),
    glm::vec3(0, 1, 0));
  blood->velocity = matRotY * matRotX * glm::vec4(0, speed, 0, 1);
  blood->rotation.z = std::uniform_real_distribution<float>(0, glm::radians(360.0f))(gamedata.random);
  blood->scale = glm::vec3(std::normal_distribution<float>(0, 1)(gamedata.random) * 0.5f + 0.75f);
  blood->gravityScale = 1;
  blood->lifespan = 0.5f;
  AddActor(blood);
#endif
}
