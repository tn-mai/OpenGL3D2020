/**
* @file Audio.h
*/
#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED
#undef APIENTRY
#include <cri_adx2le.h>
#include <vector>

/**
* ��������N���X.
*/
class Audio
{
public:
  static Audio& Instance();

  // �V�X�e���E�f�[�^����.
  bool Initialize(
    const char* acfPath, const char* dspBusName);
  void Finalize();
  bool Load(size_t slot, const char* acbPath, const char* awbPath);
  void Unload(size_t slot);
  void Update();

  // �Đ�����.
  void Play(size_t playerId, int cueId, float volume=1);
  void Stop(size_t playerId);
  void SetMasterVolume(float);
  float GetMasterVolume() const;

private:
  Audio() = default;
  ~Audio() { Finalize(); }
  Audio(const Audio&) = delete;
  Audio& operator=(const Audio&) = delete;

  // ADX2LE�ɐݒ肷��⏕�֐�.
  static void ErrorCallback(
    const CriChar8* errid, CriUint32 p1, CriUint32 p2, CriUint32* parray);
  static void* Allocate(void* obj, CriUint32 size);
  static void Deallocate(void* obj, void* ptr);

  // �f�[�^�Ǘ�.
  CriAtomExVoicePoolHn voicePool = nullptr;
  CriAtomDbasId dbas = CRIATOMDBAS_ILLEGAL_ID;
  std::vector<CriAtomExAcbHn> acbList;
  std::vector<CriAtomExPlayerHn> players;
  std::vector<CriAtomExAcbHn> cueIdToAcbMap;

  bool isInitialized = false; // �������ς݂Ȃ�true.
  float masterVolume = 1; // �S�̂̉���(����=0 �W������=1).
};

#endif // AUDIO_H_INCLUDED