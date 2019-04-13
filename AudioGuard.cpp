#include "AudioGuard.h"

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

#include <stdexcept> // for std::runtime_error

unsigned int AudioGuard::m_instanceCount{};
std::mutex AudioGuard::m_mtx{};

AudioGuard::AudioGuard()
{
  std::lock_guard<std::mutex> lock(m_mtx);

  if(m_instanceCount == 0U) {
    // we are the first instance -> initialize
    if (SDL_Init(SDL_INIT_AUDIO)) {
      throw std::runtime_error(std::string("Failed to initialize SDL: ") + SDL_GetError());
    }
  }
  ++m_instanceCount;
}

AudioGuard::~AudioGuard()
{
  std::lock_guard<std::mutex> lock(m_mtx);

  --m_instanceCount;
  if(m_instanceCount == 0U) {
    // we are the last instance -> cleanup
    SDL_Quit();
  }
}
