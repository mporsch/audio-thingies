#include "Algo.h"
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

  std::cout << "complete backward ";
  auto completeBackward = recording;
  for(auto&& samples : completeBackward.storage) {
    std::reverse(std::begin(samples), std::end(samples));
  }
  std::reverse(std::begin(completeBackward.storage), std::end(completeBackward.storage));
  playback.play(completeBackward);

  std::cout << "sample-wise backward (smoothed) ";
  auto samplewiseBackward = recording;
  for(auto&& samples : samplewiseBackward.storage) {
    std::reverse(std::begin(samples), std::end(samples));
  }
  samplewiseBackward = audio::smooth(samplewiseBackward, 20);
  playback.play(samplewiseBackward);

  std::cout << "group-wise backward (smoothed) ";
  auto&& groupwiseBackward = recording;
  std::reverse(std::begin(groupwiseBackward.storage), std::end(groupwiseBackward.storage));
  groupwiseBackward = audio::smooth(groupwiseBackward, 20);
  playback.play(groupwiseBackward);

  return EXIT_SUCCESS;
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
