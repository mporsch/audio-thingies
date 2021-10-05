#ifndef AUDIO_SEQUENCE_H
#define AUDIO_SEQUENCE_H

#include "SdlGuard.h"

#include <chrono>
#include <cstdint>
#include <iterator>
#include <list>
#include <vector>

namespace audio {

struct Metadata
{
  int sampleRate = 48000; // sampling frequency [Hz]
  uint8_t channelCount = 1; // number of channels to record in parallel (i.e. mono/stereo)
  uint16_t sampleCount = 4096; // number of samples to capture in one group
};

template<typename T>
struct SequenceIterator;

template<typename T>
struct Sequence
{
  using Samples = std::vector<T>;
  using Storage = std::list<Samples>;
  using iterator = SequenceIterator<T>;
  using const_iterator = SequenceIterator<const T>;

  Metadata metadata; ///< constant sequence metadata the samples were recorded with
  Storage storage; ///< samples in capture groups

  /// enqueue sample capture group
  void push(const uint8_t* stream, int len);
  template<typename FwdIt>
  void push(FwdIt first, FwdIt last);
  void push(Samples samples);

  /// get and remove front capture group
  /// @return  filled capture group or empty if none remaining
  std::vector<T> pop();

  /// determine playback length of all samples in recording
  std::chrono::milliseconds duration() const;

  typename Samples::reference operator[](size_t pos);
  typename Samples::const_reference operator[](size_t pos) const;

  size_t size() const;

  iterator begin();
  iterator end();

  const_iterator begin() const;
  const_iterator end() const;
};

template<typename T>
struct SequenceIterator
{
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = T;
  using difference_type = size_t;
  using pointer = T*;
  using reference = T&;

  SequenceIterator();
  SequenceIterator(Sequence<T>* seq);

  reference operator*() const;
  pointer operator->() const;

  SequenceIterator& operator++();
  SequenceIterator& operator--();

  SequenceIterator& operator++(int);
  SequenceIterator& operator--(int);

  bool operator==(const SequenceIterator& other) const;
  bool operator!=(const SequenceIterator& other) const;

  operator SequenceIterator<const T>();

private:
  Sequence<T>* seq_;
  typename Sequence<T>::Storage::iterator storage_;
  typename Sequence<T>::Samples::iterator sample_;
};

} // namespace audio

#include "AudioSequence_impl.h"

#endif // AUDIO_SEQUENCE_H
