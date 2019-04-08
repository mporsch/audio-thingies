#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

#include <cstdlib>
#include <iostream>
#include <list>
#include <vector>

namespace consts {
  using sample_format_type = float; // should match audioSpecFormat

  static const int audioSpecFrequency = 48000;
  static const SDL_AudioFormat audioSpecFormat = AUDIO_F32SYS; // should match sample_format_type
  static const uint8_t audioSpecChannels = 1;
  static const uint8_t audioSpecSilence = 0;
  static const uint16_t audioSpecSamples = 4096;
  static const uint16_t audioSpecPadding = 0;
  static const uint32_t audioSpecSize = 0;
} // namespace consts

template<typename T>
struct Recording
{
  std::list<std::vector<T>> storage;

  void push(const uint8_t* stream, int len)
  {
    const auto first = reinterpret_cast<const T*>(stream);
    const auto last = reinterpret_cast<const T*>(stream + len);

    storage.emplace_back(first, last);
  }

  std::vector<T> pop()
  {
    if (storage.empty()) {
      return {};
    }

    auto ret = std::move(storage.front());
    storage.pop_front();
    return ret;
  }

  static void record(void* userdata, uint8_t* stream, int len)
  {
    auto instance = reinterpret_cast<Recording*>(userdata);
    instance->push(stream, len);
  }

  static void play(void* userdata, uint8_t* stream, int len)
  {
    auto instance = reinterpret_cast<Recording*>(userdata);
    auto samples = instance->pop();

    const auto first = reinterpret_cast<uint8_t*>(samples.data());
    const size_t byteSize = samples.size() * sizeof(T);

    const auto writeByteSize = std::min(byteSize, static_cast<size_t>(len));

    memcpy(stream, first, writeByteSize);
    memset(stream + writeByteSize, 0, static_cast<size_t>(len) - writeByteSize);
  }
};

void printAudioDevices(int isCapture)
{
  const int numAudioDevices = SDL_GetNumAudioDevices(isCapture);

  std::cout << "Available audio " << (isCapture ? "capture" : "playback") << " devices:\n";
  for(int i = 0; i < numAudioDevices; ++i)
    std::cout << SDL_GetAudioDeviceName(i, isCapture) << "\n";
  std::cout << "\n";
}

template<typename T>
Recording<T> record()
{
  static const auto isCapture = SDL_TRUE;

  printAudioDevices(isCapture);

  Recording<T> rec;

  const SDL_AudioSpec want = {
    consts::audioSpecFrequency,  /**< DSP frequency -- samples per second */
    consts::audioSpecFormat,     /**< Audio data format */
    consts::audioSpecChannels,   /**< Number of channels: 1 mono, 2 stereo */
    consts::audioSpecSilence,    /**< Audio buffer silence value (calculated) */
    consts::audioSpecSamples,    /**< Audio buffer size in sample FRAMES (total samples divided by channel count) */
    consts::audioSpecPadding,    /**< Necessary for some compile environments */
    consts::audioSpecSize,       /**< Audio buffer size in bytes (calculated) */
    Recording<T>::record,        /**< Callback that feeds the audio device (NULL to use SDL_QueueAudio()). */
    &rec                         /**< Userdata passed to callback (ignored for NULL callbacks). */
  };

  SDL_AudioSpec have;
  const SDL_AudioDeviceID dev = SDL_OpenAudioDevice(nullptr, isCapture, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if (dev == 0) {
    std::cerr << "Failed to open audio: " << SDL_GetError() << "\n";
  } else {
    if (have.format != want.format) { /* we let this one thing change. */
      std::cerr << "Failed to get Float32 audio format\n";
    }

    static const auto pauseDisable = 0;
    SDL_PauseAudioDevice(dev, pauseDisable);

    SDL_Delay(5000); /* let the audio callback play some sound for 5 seconds. */

    SDL_CloseAudioDevice(dev);
  }

  return rec;
}

template<typename T>
void play(Recording<T> rec)
{
  static const auto isCapture = SDL_FALSE;

  printAudioDevices(isCapture);

  const SDL_AudioSpec want = {
    consts::audioSpecFrequency,  /**< DSP frequency -- samples per second */
    consts::audioSpecFormat,     /**< Audio data format */
    consts::audioSpecChannels,   /**< Number of channels: 1 mono, 2 stereo */
    consts::audioSpecSilence,    /**< Audio buffer silence value (calculated) */
    consts::audioSpecSamples,    /**< Audio buffer size in sample FRAMES (total samples divided by channel count) */
    consts::audioSpecPadding,    /**< Necessary for some compile environments */
    consts::audioSpecSize,       /**< Audio buffer size in bytes (calculated) */
    Recording<T>::play,          /**< Callback that feeds the audio device (NULL to use SDL_QueueAudio()). */
    &rec                         /**< Userdata passed to callback (ignored for NULL callbacks). */
  };

  SDL_AudioSpec have;
  const SDL_AudioDeviceID dev = SDL_OpenAudioDevice(nullptr, isCapture, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if (dev == 0) {
    std::cerr << "Failed to open audio: " << SDL_GetError() << "\n";
  } else {
    if (have.format != want.format) { /* we let this one thing change. */
      std::cerr << "Failed to get Float32 audio format\n";
    }

    static const auto pauseDisable = 0;
    SDL_PauseAudioDevice(dev, pauseDisable);

    SDL_Delay(5000); /* let the audio callback play some sound for 5 seconds. */

    SDL_CloseAudioDevice(dev);
  }
}

int main(int, char**)
{
  std::cout << "SDL_GetRevision(): " << SDL_GetRevision() << "\n";

  if (SDL_Init(SDL_INIT_AUDIO)) {
    std::cerr << "Failed to initialize SDL: " << SDL_GetError() << "\n";
    return EXIT_FAILURE;
  }

  auto rec = record<consts::sample_format_type>();
  play(std::move(rec));

  SDL_Quit();

  return EXIT_SUCCESS;
}
