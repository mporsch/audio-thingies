#ifndef AUDIO_DEVICE_H
#define AUDIO_DEVICE_H

#include "AudioSequence.h"
#include "SdlGuard.h"

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

#include <chrono>
#include <cstdint>

namespace audio {

template<typename T>
struct DeviceCapture
{
  DeviceCapture(const Metadata& metadata = Metadata());
  DeviceCapture(const DeviceCapture&) = delete;
  DeviceCapture(DeviceCapture&&) = delete;
  ~DeviceCapture();

  Sequence<T> record(std::chrono::milliseconds length);
  Sequence<T> record(uint32_t lengthMsec);

private:
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
  ~DevicePlayback();

  void play(Sequence<T> seq);

private:
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
