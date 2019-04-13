#ifndef AUDIO_GUARD_H
#define AUDIO_GUARD_H

#include <mutex> // for std::mutex

class AudioGuard
{
public:
  AudioGuard();
  AudioGuard(AudioGuard const &other) = delete;
  AudioGuard(AudioGuard &&other) = delete;
  ~AudioGuard();

  AudioGuard &operator=(AudioGuard const &other) = delete;
  AudioGuard &operator=(AudioGuard &&other) = delete;

private:
  static unsigned int m_instanceCount;
  static std::mutex m_mtx;
};

#endif // AUDIO_GUARD_H
