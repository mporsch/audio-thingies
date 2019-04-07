#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

#include <cstdlib>
#include <iostream>
#include <list>
#include <vector>

namespace consts {
  static const int audioSpecFrequency = 48000;
  static const auto audioSpecFormat = AUDIO_F32;
  static const uint8_t audioSpecChannels = 1;
  static const uint8_t audioSpecSilence = 0;
  static const uint16_t audioSpecSamples = 4096;
  static const uint16_t audioSpecPadding = 0;
  static const uint32_t audioSpecSize = 0;
} // namespace consts

struct Recording
{
  std::list<std::vector<uint8_t>> storage;

  void push(const uint8_t* stream, int len)
  {
    storage.emplace_back(std::vector<uint8_t>(stream, stream + len));
  }

  std::vector<uint8_t> pop()
  {
    auto tmp = std::move(storage.front());
    storage.pop_front();
    return tmp;
  }

  static void record(void* userdata, uint8_t* stream, int len)
  {
    auto recording = reinterpret_cast<Recording*>(userdata);
    recording->push(stream, len);
  }

  static void play(void* userdata, uint8_t* stream, int len)
  {
    auto recording = reinterpret_cast<Recording*>(userdata);
    auto streamData = recording->pop();
    memcpy(stream, streamData.data(), std::min(streamData.size(), static_cast<size_t>(len)));
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

Recording record()
{
  static const auto isCapture = SDL_TRUE;

  printAudioDevices(isCapture);

  Recording rec;

  const SDL_AudioSpec want = {
    consts::audioSpecFrequency,  /**< DSP frequency -- samples per second */
    consts::audioSpecFormat,     /**< Audio data format */
    consts::audioSpecChannels,   /**< Number of channels: 1 mono, 2 stereo */
    consts::audioSpecSilence,    /**< Audio buffer silence value (calculated) */
    consts::audioSpecSamples,    /**< Audio buffer size in sample FRAMES (total samples divided by channel count) */
    consts::audioSpecPadding,    /**< Necessary for some compile environments */
    consts::audioSpecSize,       /**< Audio buffer size in bytes (calculated) */
    Recording::record,           /**< Callback that feeds the audio device (NULL to use SDL_QueueAudio()). */
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

void play(Recording rec)
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
    Recording::play,             /**< Callback that feeds the audio device (NULL to use SDL_QueueAudio()). */
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

  auto rec = record();
  play(std::move(rec));

  SDL_Quit();

  return EXIT_SUCCESS;
}
