#ifndef AUDIO_DEVICE_IMPL_H
#define AUDIO_DEVICE_IMPL_H

#ifndef AUDIO_DEVICE_H
#error "Include via AudioDevice.h"
#endif // AUDIO_DEVICE_H

#include <cassert>
#include <iostream>
#include <stdexcept>

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

  inline void printAudioDevices(int isCapture)
  {
    const int numAudioDevices = SDL_GetNumAudioDevices(isCapture);

    std::cout << "Available audio " << (isCapture ? "capture" : "playback") << " devices:\n";
    for(int i = 0; i < numAudioDevices; ++i) {
      std::cout << SDL_GetAudioDeviceName(i, isCapture) << std::endl;
    }
  }
} // namespace detail

template<typename T>
void Sequence<T>::push(const uint8_t* stream, int len)
{
  const auto first = reinterpret_cast<const T*>(stream);
  const auto last = reinterpret_cast<const T*>(stream + len);

  push(first, last);
}

template<typename T>
template<typename FwdIt>
void Sequence<T>::push(FwdIt first, FwdIt last)
{
  storage.emplace_back(first, last);
}

template<typename T>
void Sequence<T>::push(std::vector<T> samples)
{
  storage.emplace_back(std::move(samples));
}

template<typename T>
std::vector<T> Sequence<T>::pop()
{
  if (storage.empty()) {
    return {};
  }

  auto ret = std::move(storage.front());
  storage.pop_front();
  return ret;
}


template<typename T>
std::chrono::milliseconds Sequence<T>::length() const
{
  assert(sampleRate > 0);
  return std::chrono::milliseconds((storage.size() * sampleSize) / (sampleRate / 1000));
}


template<typename T>
AudioDeviceCapture<T>::AudioDeviceCapture(int sampleRate,
                                          uint8_t channelCount,
                                          uint16_t sampleSize)
  : seq_{sampleRate, channelCount, sampleSize, {}}
{
  const SDL_AudioSpec want = {
    sampleRate,                            /**< DSP frequency -- samples per second */
    detail::FormatLookUp<T>::format,       /**< Audio data format */
    channelCount,                          /**< Number of channels: 1 mono, 2 stereo */
    detail::audioSpecSilence,              /**< Audio buffer silence value (calculated) */
    sampleSize,                            /**< Audio buffer size in sample FRAMES (total samples divided by channel count) */
    detail::audioSpecPadding,              /**< Necessary for some compile environments */
    detail::audioSpecSize,                 /**< Audio buffer size in bytes (calculated) */
    AudioDeviceCapture<T>::deviceCallback, /**< Callback that feeds the audio device (NULL to use SDL_QueueAudio()). */
    this                                   /**< Userdata passed to callback (ignored for NULL callbacks). */
  };

  static const int isCapture = SDL_TRUE;

  detail::printAudioDevices(isCapture);

  SDL_AudioSpec have;
  deviceId_ = SDL_OpenAudioDevice(nullptr, isCapture, &want, &have, detail::allowedAudioChange);
  if (!detail::isValid(deviceId_))
    throw std::runtime_error(std::string("Failed to open audio: ") + SDL_GetError());
}

template<typename T>
Sequence<T> AudioDeviceCapture<T>::record(uint32_t lengthMsec)
{
  std::cout << "recording for " << lengthMsec << "ms ..." << std::endl;

  SDL_PauseAudioDevice(deviceId_, detail::pauseDisable);

  // block here for the duration of the recording
  SDL_Delay(lengthMsec);

  SDL_PauseAudioDevice(deviceId_, detail::pauseEnable);

  return Sequence<T>{seq_.sampleRate, seq_.channelCount, seq_.sampleSize, std::move(seq_.storage)};
}

template<typename T>
void AudioDeviceCapture<T>::deviceCallback(void* userdata, uint8_t* stream, int len)
{
  auto instance = reinterpret_cast<AudioDeviceCapture<T>*>(userdata);
  instance->deviceCallback(stream, len);
}

template<typename T>
void AudioDeviceCapture<T>::deviceCallback(uint8_t* stream, int len)
{
  seq_.push(stream, len);
}


template<typename T>
AudioDevicePlayback<T>::AudioDevicePlayback(int sampleRate,
                                            uint8_t channelCount,
                                            uint16_t sampleSize)
{
  const SDL_AudioSpec want = {
    sampleRate,                             /**< DSP frequency -- samples per second */
    detail::FormatLookUp<T>::format,        /**< Audio data format */
    channelCount,                           /**< Number of channels: 1 mono, 2 stereo */
    detail::audioSpecSilence,               /**< Audio buffer silence value (calculated) */
    sampleSize,                             /**< Audio buffer size in sample FRAMES (total samples divided by channel count) */
    detail::audioSpecPadding,               /**< Necessary for some compile environments */
    detail::audioSpecSize,                  /**< Audio buffer size in bytes (calculated) */
    AudioDevicePlayback<T>::deviceCallback, /**< Callback that feeds the audio device (NULL to use SDL_QueueAudio()). */
    this                                    /**< Userdata passed to callback (ignored for NULL callbacks). */
  };

  static const int isCapture = SDL_FALSE;

  detail::printAudioDevices(isCapture);

  SDL_AudioSpec have;
  deviceId_ = SDL_OpenAudioDevice(nullptr, isCapture, &want, &have, detail::allowedAudioChange);
  if (!detail::isValid(deviceId_))
    throw std::runtime_error(std::string("Failed to open audio: ") + SDL_GetError());
}

template<typename T>
void AudioDevicePlayback<T>::play(Sequence<T> seq)
{
  seq_ = std::move(seq);

  std::cout << "playback for " << seq_.length().count() << "ms ..." << std::endl;

  SDL_PauseAudioDevice(deviceId_, detail::pauseDisable);

  // block here for the duration of the playback
  SDL_Delay(seq_.length().count());
}

template<typename T>
void AudioDevicePlayback<T>::deviceCallback(void* userdata, uint8_t* stream, int len)
{
  auto instance = reinterpret_cast<AudioDevicePlayback<T>*>(userdata);
  instance->deviceCallback(stream, len);
}

template<typename T>
void AudioDevicePlayback<T>::deviceCallback(uint8_t* stream, int len)
{
  auto samples = seq_.pop();
  if (samples.empty()) {
    SDL_PauseAudioDevice(deviceId_, detail::pauseEnable);
    return;
  }

  const auto first = reinterpret_cast<uint8_t*>(samples.data());
  const size_t byteSize = samples.size() * sizeof(T);

  const auto writeByteSize = std::min(byteSize, static_cast<size_t>(len));

  memcpy(stream, first, writeByteSize);
  memset(stream + writeByteSize, 0, static_cast<size_t>(len) - writeByteSize);
}

#endif // AUDIO_DEVICE_IMPL_H
