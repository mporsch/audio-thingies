#include "AudioDevice.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace consts {
  constexpr audio::Metadata metadata;

  // sample duration
  static const float dT = 1 / static_cast<float>(metadata.sampleRate);

  // sweep 20Hz ... 20kHz
  static const float fMin = 20.f;
  static const float fMax = 20000.f;
  static const float sweepFactor = 1.1f;
} // namespace consts

audio::Sequence<float> sweepNaive()
{
  audio::Sequence<float> seq{consts::metadata};

  // time accumulator
  float t = 0.f;

  float values[consts::metadata.sampleCount];

  for(float freq = consts::fMin; freq < consts::fMax; freq *= consts::sweepFactor) {
    for (auto&& value : values) {
      value = std::sin(2 * static_cast<float>(M_PI) * freq * t);
      t += consts::dT;
    }

    seq.push(std::begin(values), std::end(values));
  }

  return seq;
}

audio::Sequence<float> sweepTimeAccumulator()
{
  audio::Sequence<float> seq{consts::metadata};

  // time accumulator
  float t = 0.f;

  // phase continuity offset
  // recalculated on each frequency update to guarantee continuous phase and smooth audio transition
  // inspired by https://dsp.stackexchange.com/q/971
  float phiOffset = 0.f;

  float values[consts::metadata.sampleCount];

  float freq = consts::fMin;
  while (freq < consts::fMax)
  {
    // cache the last phi of this frequency
    float phi = 0.f;

    for (auto&& value : values) {
      phi = 2 * static_cast<float>(M_PI) * freq * t + phiOffset;
      value = std::sin(phi);
      t += consts::dT;
    }

    seq.push(std::begin(values), std::end(values));

    const auto nextFreq = freq * consts::sweepFactor;

    // recalculate phase continuity offset
    phiOffset = phi - nextFreq * (t - consts::dT);

    freq = nextFreq;
  }

  return seq;
}

audio::Sequence<float> sweepPhaseAccumulator()
{
  audio::Sequence<float> seq{consts::metadata};

  // phase accumulator
  // to guarantee continuous phase and smooth audio transition
  // note: mind numeric inaccuracies depending on duration and frequency
  float phi = 0.f;

  float values[consts::metadata.sampleCount];

  for(float freq = consts::fMin; freq < consts::fMax; freq *= consts::sweepFactor) {
    const auto deltaPhi = 2 * static_cast<float>(M_PI) * freq * consts::dT;

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
  audio::DevicePlayback<float> playback(consts::metadata);

  std::cout << "simple sweep..." << std::endl;
  playback.play(sweepNaive());

  std::cout << "sweep generated using time accumulator..." << std::endl;
  playback.play(sweepTimeAccumulator());

  std::cout << "sweep generated using phase accumulator..." << std::endl;
  playback.play(sweepPhaseAccumulator());

  return EXIT_SUCCESS;
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
