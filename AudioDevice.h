#ifndef AUDIO_DEVICE_H
#define AUDIO_DEVICE_H

#include "AudioGuard.h"

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

#include <chrono>
#include <cstdint>
#include <list>
#include <vector>

template<typename T>
struct Sequence
{
  int sampleRate;
  uint8_t channelCount;
  uint16_t sampleSize;
  std::list<std::vector<T>> storage;

  void push(const uint8_t* stream, int len);

  template<typename FwdIt>
  void push(FwdIt first, FwdIt last);

  std::vector<T> pop();

  std::chrono::milliseconds length() const;
};

template<typename T>
struct AudioDeviceCapture
{
  AudioDeviceCapture(int sampleRate,
                     uint8_t channelCount,
                     uint16_t sampleSize);
  AudioDeviceCapture(const AudioDeviceCapture&) = delete;
  AudioDeviceCapture(AudioDeviceCapture&&) = delete;

  Sequence<T> record(uint32_t lengthMsec);

  static void deviceCallback(void* userdata, uint8_t* stream, int len);
  void deviceCallback(uint8_t* stream, int len);

private:
  AudioGuard guard_;
  Sequence<T> seq_;
  SDL_AudioDeviceID deviceId_;
};

template<typename T>
struct AudioDevicePlayback
{
  AudioDevicePlayback(int sampleRate,
                      uint8_t channelCount,
                      uint16_t sampleSize);
  AudioDevicePlayback(const AudioDevicePlayback&) = delete;
  AudioDevicePlayback(AudioDevicePlayback&&) = delete;

  void play(Sequence<T> seq);

  static void deviceCallback(void* userdata, uint8_t* stream, int len);
  void deviceCallback(uint8_t* stream, int len);

private:
  AudioGuard guard_;
  SDL_AudioDeviceID deviceId_;
  Sequence<T> seq_;
};

#include "AudioDevice_impl.h"

#endif // AUDIO_DEVICE_H
