#include "SdlGuard.h"

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

#include <stdexcept> // for std::runtime_error

namespace audio {

unsigned int SdlGuard::m_instanceCount{};
std::mutex SdlGuard::m_mtx{};

SdlGuard::SdlGuard()
{
  std::lock_guard<std::mutex> lock(m_mtx);

  if(m_instanceCount == 0U) {
    // we are the first instance -> initialize
    if(SDL_Init(SDL_INIT_AUDIO)) {
      throw std::runtime_error(std::string("Failed to initialize SDL: ") + SDL_GetError());
    }
  }
  ++m_instanceCount;
}

SdlGuard::~SdlGuard()
{
  std::lock_guard<std::mutex> lock(m_mtx);

  --m_instanceCount;
  if(m_instanceCount == 0U) {
    // we are the last instance -> cleanup
    SDL_Quit();
  }
}

} // namespace audio
