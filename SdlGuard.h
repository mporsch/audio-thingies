#ifndef SDL_AUDIO_GUARD_H
#define SDL_AUDIO_GUARD_H

#include <mutex> // for std::mutex

namespace audio {

class SdlGuard
{
public:
  SdlGuard();
  SdlGuard(SdlGuard const &other) = delete;
  SdlGuard(SdlGuard &&other) = delete;
  ~SdlGuard();

  SdlGuard &operator=(SdlGuard const &other) = delete;
  SdlGuard &operator=(SdlGuard &&other) = delete;

private:
  static unsigned int m_instanceCount;
  static std::mutex m_mtx;
};

} // namespace audio

#endif // SDL_AUDIO_GUARD_H
