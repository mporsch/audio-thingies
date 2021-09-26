#include "AudioDevice.h"

#include <algorithm>
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

  std::cout << "forward ";
  playback.play(recording);

  for(auto&& samples : recording.storage) {
    std::reverse(std::begin(samples), std::end(samples));
  }
  std::reverse(std::begin(recording.storage), std::end(recording.storage));

  std::cout << "backward ";
  playback.play(recording);

  return EXIT_SUCCESS;
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
