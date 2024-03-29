#ifndef AUDIO_DEVICE_IMPL_H
#define AUDIO_DEVICE_IMPL_H

#ifndef AUDIO_DEVICE_H
#error "Include via AudioDevice.h"
#endif // AUDIO_DEVICE_H

#include <iostream>
#include <stdexcept>

namespace audio {

namespace detail {
  static const uint8_t audioSpecSilence = 0;
  static const uint16_t audioSpecPadding = 0;
  static const uint32_t audioSpecSize = 0;

  static const int allowedAudioChange = 0;

  static const int pauseEnable = 1;
  static const int pauseDisable = 0;

  template<typename T>
  struct FormatLookUp
  {
  };

  template<>
  struct FormatLookUp<float>
  {
    static const SDL_AudioFormat format = AUDIO_F32SYS;
  };

  inline bool isValid(SDL_AudioDeviceID deviceId)
  {
    return (deviceId >= 2); // see SDL_OpenAudioDevice()
  }

  inline void printDevices(int isCapture)
  {
    const int numAudioDevices = SDL_GetNumAudioDevices(isCapture);

    std::cout << "Available audio " << (isCapture ? "capture" : "playback") << " devices:\n";
    for(int i = 0; i < numAudioDevices; ++i) {
      std::cout << SDL_GetAudioDeviceName(i, isCapture) << std::endl;
    }
  }
} // namespace detail

template<typename T>
DeviceCapture<T>::DeviceCapture(const Metadata& metadata)
  : seq_{metadata, {}}
{
  const SDL_AudioSpec want = {
    metadata.sampleRate,                   /**< DSP frequency -- samples per second */
    detail::FormatLookUp<T>::format,       /**< Audio data format */
    metadata.channelCount,                 /**< Number of channels: 1 mono, 2 stereo */
    detail::audioSpecSilence,              /**< Audio buffer silence value (calculated) */
    metadata.sampleCount,                  /**< Audio buffer size in sample FRAMES (total samples divided by channel count) */
    detail::audioSpecPadding,              /**< Necessary for some compile environments */
    detail::audioSpecSize,                 /**< Audio buffer size in bytes (calculated) */
    DeviceCapture<T>::deviceCallback,      /**< Callback that feeds the audio device (NULL to use SDL_QueueAudio()). */
    this                                   /**< Userdata passed to callback (ignored for NULL callbacks). */
  };

  static const int isCapture = SDL_TRUE;

  detail::printDevices(isCapture);

  SDL_AudioSpec have;
  deviceId_ = SDL_OpenAudioDevice(nullptr, isCapture, &want, &have, detail::allowedAudioChange);
  if(!detail::isValid(deviceId_))
    throw std::runtime_error(std::string("Failed to open audio: ") + SDL_GetError());
}

template<typename T>
DeviceCapture<T>::~DeviceCapture()
{
  SDL_CloseAudioDevice(deviceId_);
}

template<typename T>
Sequence<T> DeviceCapture<T>::record(std::chrono::milliseconds length)
{
  using Msec = std::chrono::duration<uint32_t, std::milli>;
  return record(std::chrono::duration_cast<Msec>(length).count());
}

template<typename T>
Sequence<T> DeviceCapture<T>::record(uint32_t lengthMsec)
{
  std::cout << "recording for " << lengthMsec << "ms ..." << std::endl;

  SDL_PauseAudioDevice(deviceId_, detail::pauseDisable);

  // block here for the duration of the recording
  SDL_Delay(lengthMsec);

  SDL_PauseAudioDevice(deviceId_, detail::pauseEnable);

  return Sequence<T>{seq_.metadata, std::move(seq_.storage)};
}

template<typename T>
void DeviceCapture<T>::deviceCallback(void* userdata, uint8_t* stream, int len)
{
  auto instance = reinterpret_cast<DeviceCapture<T>*>(userdata);
  instance->deviceCallback(stream, len);
}

template<typename T>
void DeviceCapture<T>::deviceCallback(uint8_t* stream, int len)
{
  seq_.push(stream, len);
}


template<typename T>
DevicePlayback<T>::DevicePlayback(const Metadata& metadata)
{
  const SDL_AudioSpec want = {
    metadata.sampleRate,                    /**< DSP frequency -- samples per second */
    detail::FormatLookUp<T>::format,        /**< Audio data format */
    metadata.channelCount,                  /**< Number of channels: 1 mono, 2 stereo */
    detail::audioSpecSilence,               /**< Audio buffer silence value (calculated) */
    metadata.sampleCount,                   /**< Audio buffer size in sample FRAMES (total samples divided by channel count) */
    detail::audioSpecPadding,               /**< Necessary for some compile environments */
    detail::audioSpecSize,                  /**< Audio buffer size in bytes (calculated) */
    DevicePlayback<T>::deviceCallback,      /**< Callback that feeds the audio device (NULL to use SDL_QueueAudio()). */
    this                                    /**< Userdata passed to callback (ignored for NULL callbacks). */
  };

  static const int isCapture = SDL_FALSE;

  detail::printDevices(isCapture);

  SDL_AudioSpec have;
  deviceId_ = SDL_OpenAudioDevice(nullptr, isCapture, &want, &have, detail::allowedAudioChange);
  if(!detail::isValid(deviceId_))
    throw std::runtime_error(std::string("Failed to open audio: ") + SDL_GetError());
}

template<typename T>
DevicePlayback<T>::~DevicePlayback()
{
  SDL_CloseAudioDevice(deviceId_);
}

template<typename T>
void DevicePlayback<T>::play(Sequence<T> seq)
{
  seq_ = std::move(seq);

  std::cout << "playback for " << seq_.duration().count() << "ms ..." << std::endl;

  SDL_PauseAudioDevice(deviceId_, detail::pauseDisable);

  // block here for the duration of the playback
  SDL_Delay(seq_.duration().count());
}

template<typename T>
void DevicePlayback<T>::deviceCallback(void* userdata, uint8_t* stream, int len)
{
  auto instance = reinterpret_cast<DevicePlayback<T>*>(userdata);
  instance->deviceCallback(stream, len);
}

template<typename T>
void DevicePlayback<T>::deviceCallback(uint8_t* stream, int len)
{
  auto samples = seq_.pop();
  if(samples.empty()) {
    SDL_PauseAudioDevice(deviceId_, detail::pauseEnable);
    return;
  }

  const auto first = reinterpret_cast<uint8_t*>(samples.data());
  const size_t byteSize = samples.size() * sizeof(T);

  const auto writeByteSize = std::min(byteSize, static_cast<size_t>(len));

  memcpy(stream, first, writeByteSize);
  memset(stream + writeByteSize, 0, static_cast<size_t>(len) - writeByteSize);
}

} // namespace audio

#endif // AUDIO_DEVICE_IMPL_H
