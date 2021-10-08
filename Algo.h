#ifndef AUDIO_ALGO_H
#define AUDIO_ALGO_H

#include <cassert>
#include <cstdlib>

namespace audio {

template<typename Container>
Container smooth(
    Container container,
    size_t windowRadius)
{
  const auto windowSize = windowRadius * 2 + 1;

  Container smoothed = container;

  if(windowSize < container.size()) {
    struct Window
    {
      size_t front;
      size_t mid;
      size_t back;
      size_t count;
      float sum;
    } window = {};

    // preload
    for(; window.back < windowRadius; ++window.back) {
      window.sum += container[window.back];
      ++window.count;
    }
    // front half window
    for(; window.back < windowSize; ++window.mid, ++window.back) {
      window.sum += container[window.back];
      ++window.count;
      smoothed[window.mid] = window.sum / window.count;
    }
    // whole window
    for(; window.back < container.size(); ++window.front, ++window.mid, ++window.back) {
      window.sum -= container[window.front];
      window.sum += container[window.back];
      smoothed[window.mid] = window.sum / window.count;
    }
    // back half window
    for(; window.mid < container.size(); ++window.front, ++window.mid) {
      window.sum -= container[window.front];
      --window.count;
      smoothed[window.mid] = window.sum / window.count;
    }
    // unload (omitted)
  } else {
    assert(false); // TODO
  }

  return smoothed;
}

} // namespace audio

#endif // AUDIO_ALGO_H
