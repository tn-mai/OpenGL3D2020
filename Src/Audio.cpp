/**
* @file Audio.cpp
*/
#include "Audio.h"
#include "Audio/OpenGL3D2020_acf.h"
#include <algorithm>
#include <iostream>

const int audioMaxVirtualVoice = 64;
const int audioMaxVoice = audioMaxVirtualVoice / 2;
const int audioMaxLoader = audioMaxVoice + 8;

/**
* �I�[�f�B�I�V�X�e���p�G���[�R�[���o�b�N.
*/
void Audio::ErrorCallback(const CriChar8* errid, CriUint32 p1, CriUint32 p2,
  CriUint32* parray)
{
  const CriChar8* err = criErr_ConvertIdToMessage(errid, p1, p2);
  std::cerr << err << std::endl;
}

/**
* �I�[�f�B�I�V�X�e���p�A���P�[�^.
*/
void* Audio::Allocate(void* obj, CriUint32 size)
{
  return operator new(size);
}

/**
* �I�[�f�B�I�V�X�e���p�f�A���P�[�^.
*/
void Audio::Deallocate(void* obj, void* ptr)
{
  operator delete(obj);
}

/**
* ��������N���X���擾����.
*
* @return ��������N���X�̃C���X�^���X�̎Q��.
*/
Audio& Audio::Instance()
{
  static Audio instance;
  return instance;
}

/**
* ��������V�X�e��������������.
*
* @param acfPaht    �S�̐ݒ��ێ�����ACF�t�@�C���̃p�X.
* @param dspBusName �����V�X�e���Ŏg�p����DSP�o�X��.
*
* @retval true  ����������.
* @retval false ���������s.
*/
bool Audio::Initialize(const char* acfPath, const char* dspBusName)
{
  // �������ς݂Ȃ牽�����Ȃ�.
  if (isInitialized) {
    return true;
  }

  // �G���[�R�[���o�b�N�ƃ������Ǘ��֐���o�^����.
  criErr_SetCallback(ErrorCallback);
  criAtomEx_SetUserAllocator(Allocate, Deallocate, nullptr);

  // �������p�����[�^��ݒ肵��ADX2 LE������������.
  CriFsConfig fsConfig;
  criFs_SetDefaultConfig(&fsConfig);
  // �X�g���[���Đ���L���ɂ���ꍇ�Anum_voices���傫�����邱��.
  // �f�t�H���g�ł�num_voices��8�Anum_loaders��16�Ȃ̂ŁAnum_voices+8����΂悳����.
  // ������������ł�+1�ł����������A�����̐ݒ�ő������邩������Ȃ��̂�+8�����.
  // max_virtual_voices�̃f�t�H���g��16�Ȃ̂ł���ƍ��킹��Ă����邪�A�ǂ��炪�����Ȃ̂�...
  // �X�g���[���Đ���L���ɂ���Ƃ��ׂẴ{�C�X�Ƀ��[�_�[�����蓖�Ă��邽��.
  fsConfig.num_loaders = audioMaxLoader;
  CriAtomExConfig_WASAPI libConfig;
  criAtomEx_SetDefaultConfig_WASAPI(&libConfig);
  libConfig.atom_ex.fs_config = &fsConfig;
  // ��������\�ȉ����̍ő吔. ���ۂ̔������̓{�C�X�v�[����num_voices�Ŏw�肷��.
  libConfig.atom_ex.max_virtual_voices = audioMaxVirtualVoice;
  // �E����W�n���w��.
  libConfig.atom_ex.coordinate_system = CRIATOMEX_COORDINATE_SYSTEM_RIGHT_HANDED;
  criAtomEx_Initialize_WASAPI(&libConfig, nullptr, 0);
  dbas = criAtomDbas_Create(nullptr, nullptr, 0);

  // �ݒ�t�@�C����ǂݍ���.
  if (criAtomEx_RegisterAcfFile(nullptr, acfPath, nullptr, 0) == CRI_FALSE) {
    std::cerr << "[�G���[]" << __func__ << ":" << acfPath << "��ǂݍ��߂܂���.\n";
    Finalize();
    return false;
  }
  criAtomEx_AttachDspBusSetting(dspBusName, nullptr, 0);

  // �Đ�����ݒ肷��.
  CriAtomExStandardVoicePoolConfig svpConfig;
  criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&svpConfig);
  svpConfig.num_voices = audioMaxVoice; // ����������.
  svpConfig.player_config.streaming_flag = CRI_TRUE; // �X�g���[�~���O�Đ���L����.
  svpConfig.player_config.max_sampling_rate =
    CRIATOM_DEFAULT_INPUT_MAX_SAMPLING_RATE * 2; // �ő�T���v�����O���[�g. �s�b�`�ύX���l������CD������2�{��ݒ�.
  voicePool = criAtomExVoicePool_AllocateStandardVoicePool(&svpConfig, nullptr, 0);
  if (!voicePool) {
    std::cerr << "[�G���[]" << __func__ << ":�{�C�X�v�[���̍쐬�Ɏ��s.\n";
    Finalize();
    return false;
  }

  // �Đ�����p�v���C���[���쐬����.
  players.resize(8);
  for (auto& e : players) {
    e = criAtomExPlayer_Create(nullptr, nullptr, 0);
  }

  // acb�ǂݍ��݃o�b�t�@���m��.
  // �����Ŏw�肵�����́A�����ɓǂݍ��݉\��ACB�t�@�C���̍ő吔�ɂȂ�.
  // Load�Ńo�b�t�@�ɓǂݍ��݁AUnload�Ŕj������.
  acbList.resize(16);

  // �L���[ID��acb�t�@�C���̑Ή��\.
  // �L���[Id��Y�����Ɏg�����ƂőΉ�����acb�t�@�C�����擾�ł���.
  // �T�E���h�Đ��ɂ͂̓L���[ID��acb�t�@�C���̃y�A���K�v�Ȃ���.
  // Load�őΉ��Â����s���AUnload�ŉ��������.
  cueIdToAcbMap.resize(4096);

  isInitialized = true;
  return true;
}

/**
* ��������V�X�e����j������.
*/
void Audio::Finalize()
{
  // ���ׂẴv���C���[��j��.
  for (auto& e : players) {
    if (e) {
      criAtomExPlayer_Destroy(e);
    }
  }
  players.clear();

  // ���ׂĂ�ACB�t�@�C����j��.
  for (auto e : acbList) {
    if (e) {
      criAtomExAcb_Release(e);
    }
  }
  acbList.clear();

  // �L���[ID��ACB�̑Ή��\��������.
  cueIdToAcbMap.clear();

  // �{�C�X�v�[����j��.
  if (voicePool) {
    criAtomExVoicePool_Free(voicePool);
    voicePool = nullptr;
  }

  // ACF�t�@�C���̓o�^������.
  criAtomEx_UnregisterAcf();

  // DBAS��j��.
  if (dbas != CRIATOMDBAS_ILLEGAL_ID) {
    criAtomDbas_Destroy(dbas);
    dbas = CRIATOMDBAS_ILLEGAL_ID;
  }

  // ADX2LE���I��.
  criAtomEx_Finalize_WASAPI();

  isInitialized = false;
}

/**
* �I�[�f�B�I�t�@�C����ǂݍ���.
*
* @param index �ǂݍ��ݐ�I�[�f�B�I�X���b�g�ԍ�.
*
* @retval true  �ǂݍ��ݐ���.
* @retval false �ǂݍ��ݎ��s.
*/
bool Audio::Load(size_t index, const char* acbPath, const char* awbPath)
{
  if (index >= acbList.size()) {
    std::cerr << "[�G���[]" << __func__ << "�C���f�b�N�X���傫�����܂�.\n";
    return false;
  }

  Unload(index);

  acbList[index] = criAtomExAcb_LoadAcbFile(nullptr, acbPath, nullptr, awbPath, nullptr, 0);
  if (!acbList[index]) {
    std::cerr << "[�G���[]" << __func__ << ":" << acbPath << "�̓ǂݍ��݂Ɏ��s.\n";
    return false;
  }

  // �L���[ID��ACB�t�@�C���̑Ή��\���쐬����.
  const CriSint32 numCues = criAtomExAcb_GetNumCues(acbList[index]);
  for (int i = 0; i < numCues; ++i) {
    CriAtomExCueInfo cueInfo;
    if (!criAtomExAcb_GetCueInfoByIndex(acbList[index], i, &cueInfo)) {
      std::cerr << "[�G���[]" << __func__ << ":" << acbPath << "��" << i << "�Ԗڂ̃L���[�����擾�ł��܂���.\n";
      continue;
    }
    if (cueIdToAcbMap.size() <= static_cast<size_t>(cueInfo.id)) {
      cueIdToAcbMap.resize(cueInfo.id + 1);
    }
    cueIdToAcbMap[cueInfo.id] = acbList[index];
  }
  return true;
}

/**
* �I�[�f�B�I�t�@�C����j������.
*
* @param index �j������I�[�f�B�I�X���b�g�ԍ�.
*/
void Audio::Unload(size_t index)
{
  if (index >= acbList.size()) {
    std::cerr << "[�G���[]" << __func__ << "�C���f�b�N�X���傫�����܂�.\n";
    return;
  }

  if (acbList[index]) {
    std::replace(cueIdToAcbMap.begin(), cueIdToAcbMap.end(),
      acbList[index], static_cast<CriAtomExAcbHn>(nullptr));
    criAtomExAcb_Release(acbList[index]);
    acbList[index] = nullptr;
  }
}

/**
* �����V�X�e���̏�Ԃ��X�V����.
*/
void Audio::Update()
{
  if (!isInitialized) {
    return;
  }
  // �����V�X�e���̏�Ԃ��X�V.
  criAtomEx_ExecuteMain();
}

/**
* �������Đ�����.
*
* @param playerId �Đ��Ɏg�p����v���C���[�ԍ�.
* @param cueId    �Đ�����L���[ID.
*/
void Audio::Play(size_t playerId, int cueId, float volume)
{
  // �v���C���[�ԍ����v���C���[���ȏ�̏ꍇ�͉������Ȃ�.
  if (playerId >= players.size()) {
    return;
  }
  // �Ή��\��nullptr�̏ꍇ�͉������Ȃ�.
  if (!cueIdToAcbMap[cueId]) {
    return;
  }

  // �v���C���[�ɃL���[���Z�b�g.
  criAtomExPlayer_SetCueId(players[playerId], cueIdToAcbMap[cueId], cueId);

  criAtomExPlayer_SetVolume(players[playerId], volume);

  // �Z�b�g�����L���[���Đ�.
  criAtomExPlayer_Start(players[playerId]);
}

/**
* �������~����.
*
* @param playerId �Đ����~����v���C���[�ԍ�.
*/
void Audio::Stop(size_t playerId)
{
  // �v���C���[�ԍ����v���C���[���ȏ�̏ꍇ�͉������Ȃ�.
  if (playerId >= players.size()) {
    return;
  }
  // �Đ����~����.
  criAtomExPlayer_Stop(players[playerId]);
}

/**
* �S�̉��ʂ�ݒ肷��.
*
* @param volume �ݒ肷��S�̉���(����=0 �W������=1).
*/
void Audio::SetMasterVolume(float volume)
{
  if (!isInitialized) {
    return;
  }
  masterVolume = volume;
  criAtomExCategory_SetVolumeById(CRI_OPENGL3D2020_ACF_CATEGORY_CATEGORY_0, masterVolume);
}

/**
* �S�̉��ʂ��擾����.
*
* @return �ݒ肳��Ă���S�̉���(����=1 �W������=1).
*/
float Audio::GetMasterVolume() const
{
  return masterVolume;
}

