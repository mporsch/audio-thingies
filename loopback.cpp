#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace consts {
  static const int audioSpecFrequency = 48000;
  static const SDL_AudioFormat audioSpecFormat = AUDIO_F32SYS;
  static const uint8_t audioSpecChannels = 1;
  static const uint8_t audioSpecSilence = 0;
  static const uint16_t audioSpecSamples = 4096;
  static const uint16_t audioSpecPadding = 0;
  static const uint32_t audioSpecSize = 0;

  static const int allowedAudioChange = 0;

  static const auto pauseDisable = 0;

  static const uint32_t recordLengthMsec = 5000;
} // namespace consts

bool isValid(SDL_AudioDeviceID deviceId)
{
  return (deviceId >= 2); // see SDL_OpenAudioDevice()
}

void printAudioDevices(int isCapture)
{
  const int numAudioDevices = SDL_GetNumAudioDevices(isCapture);

  std::cout << "Available audio " << (isCapture ? "capture" : "playback") << " devices:\n";
  for(int i = 0; i < numAudioDevices; ++i)
    std::cout << SDL_GetAudioDeviceName(i, isCapture) << "\n";
  std::cout << "\n";
}

SDL_AudioDeviceID setupPlayDevice()
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
    nullptr,                     /**< Callback that feeds the audio device (NULL to use SDL_QueueAudio()). */
    nullptr                      /**< Userdata passed to callback (ignored for NULL callbacks). */
  };

  SDL_AudioSpec have;
  const auto deviceId = SDL_OpenAudioDevice(nullptr, isCapture, &want, &have, consts::allowedAudioChange);
  if (!isValid(deviceId))
    throw std::runtime_error(std::string("Failed to open audio: ") + SDL_GetError());

  return deviceId;
}

void captureCallback(void* userdata, uint8_t* stream, int len)
{
  auto playDeviceId = *reinterpret_cast<const SDL_AudioDeviceID*>(userdata);

  if (SDL_QueueAudio(playDeviceId, stream, static_cast<uint32_t>(len)))
    throw std::runtime_error(std::string("Failed to queue audio: ") + SDL_GetError());
}

SDL_AudioDeviceID setupCaptureDevice(SDL_AudioDeviceID* playDeviceId)
{
  static const auto isCapture = SDL_TRUE;

  printAudioDevices(isCapture);

  const SDL_AudioSpec want = {
    consts::audioSpecFrequency,  /**< DSP frequency -- samples per second */
    consts::audioSpecFormat,     /**< Audio data format */
    consts::audioSpecChannels,   /**< Number of channels: 1 mono, 2 stereo */
    consts::audioSpecSilence,    /**< Audio buffer silence value (calculated) */
    consts::audioSpecSamples,    /**< Audio buffer size in sample FRAMES (total samples divided by channel count) */
    consts::audioSpecPadding,    /**< Necessary for some compile environments */
    consts::audioSpecSize,       /**< Audio buffer size in bytes (calculated) */
    captureCallback,             /**< Callback that feeds the audio device (NULL to use SDL_QueueAudio()). */
    playDeviceId                 /**< Userdata passed to callback (ignored for NULL callbacks). */
  };

  SDL_AudioSpec have;
  const auto deviceId = SDL_OpenAudioDevice(nullptr, isCapture, &want, &have, consts::allowedAudioChange);
  if (!isValid(deviceId))
    throw std::runtime_error(std::string("Failed to open audio: ") + SDL_GetError());

  return deviceId;
}

int main(int, char**)
try {
  std::cout << "SDL_GetRevision(): " << SDL_GetRevision() << "\n";

  if (SDL_Init(SDL_INIT_AUDIO))
    throw std::runtime_error(std::string("Failed to initialize SDL: ") + SDL_GetError());

  auto playDeviceId = setupPlayDevice();
  const auto captureDeviceId = setupCaptureDevice(&playDeviceId);

  SDL_PauseAudioDevice(playDeviceId, consts::pauseDisable);
  SDL_PauseAudioDevice(captureDeviceId, consts::pauseDisable);

  SDL_Delay(consts::recordLengthMsec);

  SDL_CloseAudioDevice(captureDeviceId);
  SDL_CloseAudioDevice(playDeviceId);

  SDL_Quit();

  return EXIT_SUCCESS;
} catch(const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
