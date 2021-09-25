#include "AudioDevice.h"

#include <cstdlib>
#include <iostream>

namespace consts {
  static const uint32_t recordLengthMsec = 5000;
} // namespace consts

int main(int, char**)
try {
  audio::DeviceCapture<float> capture;
  auto recording = capture.record(consts::recordLengthMsec);

  audio::DevicePlayback<float> playback(recording.metadata);
  playback.play(std::move(recording));

  return EXIT_SUCCESS;
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
