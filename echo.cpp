#include "AudioDevice.h"

#include <cstdlib>
#include <iostream>

namespace consts {
  static const int audioSpecFrequency = 48000;
  static const uint8_t audioSpecChannels = 1;
  static const uint16_t audioSpecSamples = 4096;

  static const uint32_t recordLengthMsec = 5000;
} // namespace consts

int main(int, char**)
try {
  AudioDeviceCapture<float> capture(consts::audioSpecFrequency, consts::audioSpecChannels, consts::audioSpecSamples);
  auto recording = capture.record(consts::recordLengthMsec);

  AudioDevicePlayback<float> playback(consts::audioSpecFrequency, consts::audioSpecChannels, consts::audioSpecSamples);
  playback.play(std::move(recording));

  return EXIT_SUCCESS;
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
