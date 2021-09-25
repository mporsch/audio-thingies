#ifndef AUDIO_DEVICE_H
#define AUDIO_DEVICE_H

#include "SdlGuard.h"

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

#include <chrono>
#include <cstdint>
#include <list>
#include <vector>

namespace audio {

struct Metadata
{
  int sampleRate = 48000; // sampling frequency [Hz]
  uint8_t channelCount = 1; // number of channels to record in parallel (i.e. mono/stereo)
  uint16_t sampleCount = 4096; // number of samples to capture in one group
};

template<typename T>
struct Sequence
{
  Metadata metadata;
  std::list<std::vector<T>> storage;

  void push(const uint8_t* stream, int len);

  template<typename FwdIt>
  void push(FwdIt first, FwdIt last);

  void push(std::vector<T> sample);

  std::vector<T> pop();

  std::chrono::milliseconds length() const;
};

template<typename T>
struct DeviceCapture
{
  DeviceCapture(const Metadata& metadata = Metadata());
  DeviceCapture(const DeviceCapture&) = delete;
  DeviceCapture(DeviceCapture&&) = delete;

  Sequence<T> record(std::chrono::milliseconds length);
  Sequence<T> record(uint32_t lengthMsec);

  static void deviceCallback(void* userdata, uint8_t* stream, int len);
  void deviceCallback(uint8_t* stream, int len);

private:
  SdlGuard guard_;
  Sequence<T> seq_;
  SDL_AudioDeviceID deviceId_;
};

template<typename T>
struct DevicePlayback
{
  DevicePlayback(const Metadata& metadata = Metadata());
  DevicePlayback(const DevicePlayback&) = delete;
  DevicePlayback(DevicePlayback&&) = delete;

  void play(Sequence<T> seq);

  static void deviceCallback(void* userdata, uint8_t* stream, int len);
  void deviceCallback(uint8_t* stream, int len);

private:
  SdlGuard guard_;
  SDL_AudioDeviceID deviceId_;
  Sequence<T> seq_;
};

} // namespace audio

#include "AudioDevice_impl.h"

#endif // AUDIO_DEVICE_H
