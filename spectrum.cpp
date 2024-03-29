#include "Algo.h"
#include "AudioDevice.h"

#include "kissfft/kissfft.hh"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>

//#define DEBUG_SINE_FREQUENCY 500.0F

namespace consts {
  static const std::chrono::milliseconds recordLength(2000);
} // namespace consts

audio::Sequence<float> sineSequence(float freq, std::chrono::seconds length)
{
  audio::Sequence<float> seq;

  // time accumulator
  float t = 0.f; // [s]
  const float dT = 1.0F / seq.metadata.sampleCount; // [s]

  const auto sampleCount = seq.metadata.sampleRate * length.count() / seq.metadata.sampleCount;
  for(uint32_t i = 0U; i < sampleCount; ++i) {
    std::vector<float> values(seq.metadata.sampleCount);
    for(auto&& value : values) {
      value = std::sin(2 * static_cast<float>(M_PI) * freq * t);
      t += dT;
    }
    seq.push(std::move(values));
  }

  return seq;
}

std::vector<float> fft(const audio::Sequence<float>& seq)
{
  const size_t halfSize = seq.metadata.sampleCount / 2;

  std::vector<float> ret(halfSize);

  std::vector<kissfft<float>::cpx_t> transformedSum(halfSize);
  {
    kissfft<float> calc(halfSize, false);

    (void)std::for_each(
          std::begin(seq.storage), std::end(seq.storage),
          [&](const std::vector<float>& samples) {
      // calculate FFT (complex) for (real) samples
      std::vector<kissfft<float>::cpx_t> transformed(halfSize);
      calc.transform_real(samples.data(), transformed.data());

      // sum up (complex)
      (void)std::transform(
            std::begin(transformed), std::end(transformed),
            std::begin(transformedSum), std::begin(transformedSum),
            std::plus<kissfft<float>::cpx_t>());
    });
  }

  // calculate absolute of complex FFT results
  (void)std::transform(
        std::begin(transformedSum), std::end(transformedSum),
        std::begin(ret),
        [](const kissfft<float>::cpx_t& v) -> float { return std::abs(v); });

  return ret;
}

void analyze(const std::vector<float>& spectrum, const audio::Metadata& metadata)
{
  // filter some minor peaks
  const auto smoothedSpectrum = audio::smooth(spectrum, 5);

  static const float thresh = 10.f;

  bool wasRising = true;
  float min = 0.f;
  float max = 0.f;
  auto pos = std::begin(smoothedSpectrum);
  while (pos != std::end(smoothedSpectrum)) {
    pos = std::adjacent_find(
          pos, std::end(smoothedSpectrum),
          [&](float lhs, float rhs) -> bool {
      const bool isRising = (lhs < rhs);

      if(wasRising && !isRising) { // peak
        wasRising = isRising;
        max = lhs;
        if(max - min > thresh) {
          return true;
        }
      } else if(!wasRising && isRising) { // valley
        wasRising = isRising;
        if(max - min > thresh)
          min = lhs;
      }
      return false;
    });

    auto peakOffset = std::distance(std::begin(smoothedSpectrum), pos);

    // frequency calculation from spectrum position
    // inspired by https://stackoverflow.com/a/4230658
    auto peakFreq = metadata.sampleRate * peakOffset / metadata.sampleCount;

    std::cout << "spectrum peak at " << peakFreq << "Hz" << std::endl;
  }
}

int main(int, char**)
try {
  // record / generate
#ifndef DEBUG_SINE_FREQUENCY
  audio::DeviceCapture<float> capture;
  auto seq = capture.record(consts::recordLength);
#else
  auto seq = sineSequence(
        DEBUG_SINE_FREQUENCY,
        std::chrono::duration_cast<std::chrono::seconds>(
          consts::recordLength));
#endif // DEBUG_SINE_FREQUENCY

  // play back
  audio::DevicePlayback<float> play(seq.metadata);
  play.play(seq);

  // calculate spectrum
  const auto spectrum = fft(seq);

  // print spectrum characteristics
  analyze(spectrum, seq.metadata);

  return EXIT_SUCCESS;
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
