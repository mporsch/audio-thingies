#include "AudioDevice.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace consts {
  static const int audioSpecFrequency = 48000;
  static const uint8_t audioSpecChannels = 1;
  static const uint16_t audioSpecSamples = 4096;

  static const auto sampleDuration = 1 / static_cast<float>(audioSpecFrequency);

  static const float sweepFactor = 1.1f;
} // namespace consts

Sequence<float> sweepTimeAccumulator()
{
  Sequence<float> seq;
  seq.sampleRate = consts::audioSpecFrequency;
  seq.sampleSize = consts::audioSpecSamples;
  seq.channelCount = consts::audioSpecChannels;

  // time accumulator
  float t = 0.f;

  // phase continuity offset
  // recalculated on each frequency update to guarantee continuous phase and smooth audio transition
  float phiOffset = 0.f;

  float values[consts::audioSpecSamples];

  // sweep 20Hz ... 20kHz
  float freq = 20.f;
  while (freq < 20000.f)
  {
    // cache the last phi of this frequency
    float phi = 0.f;

    for (auto&& value : values) {
      phi = freq * t + phiOffset;
      value = std::sin(phi);
      t += consts::sampleDuration;
    }

    seq.push(std::begin(values), std::end(values));

    const auto nextFreq = freq * consts::sweepFactor;

    // recalculate phase continuity offset
    phiOffset = phi - nextFreq * (t - consts::sampleDuration);

    freq = nextFreq;
  }

  return seq;
}

Sequence<float> sweepPhaseAccumulator()
{
  Sequence<float> seq;
  seq.sampleRate = consts::audioSpecFrequency;
  seq.sampleSize = consts::audioSpecSamples;
  seq.channelCount = consts::audioSpecChannels;

  // phase accumulator
  // to guarantee continuous phase and smooth audio transition
  // note: mind numeric inaccuracies depending on duration and frequency
  float phi = 0.f;

  float values[consts::audioSpecSamples];

  // sweep 20Hz ... 20kHz
  for(float freq = 20.f; freq < 20000.f; freq *= consts::sweepFactor) {
    const auto deltaPhi = freq * consts::sampleDuration;

    for (auto&& value : values) {
      value = std::sin(phi);
      phi += deltaPhi;
    }

    seq.push(std::begin(values), std::end(values));
  }

  return seq;
}

int main(int, char**)
try {
  AudioDevicePlayback<float> playback(consts::audioSpecFrequency, consts::audioSpecChannels, consts::audioSpecSamples);

  std::cout << "sweep generated using time accumulator...\n";
  playback.play(sweepTimeAccumulator());

  std::cout << "sweep generated using phase accumulator...\n";
  playback.play(sweepPhaseAccumulator());

  return EXIT_SUCCESS;
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
