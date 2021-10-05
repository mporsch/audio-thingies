#ifndef AUDIO_SEQUENCE_IMPL_H
#define AUDIO_SEQUENCE_IMPL_H

#ifndef AUDIO_SEQUENCE_H
#error "Include via AudioSequence.h"
#endif // AUDIO_SEQUENCE_H

#include <cassert>

namespace audio {

template<typename T>
void Sequence<T>::push(const uint8_t* stream, int len)
{
  const auto first = reinterpret_cast<const T*>(stream);
  const auto last = reinterpret_cast<const T*>(stream + len);

  push(first, last);
}

template<typename T>
template<typename FwdIt>
void Sequence<T>::push(FwdIt first, FwdIt last)
{
  storage.emplace_back(first, last);
}

template<typename T>
void Sequence<T>::push(Samples samples)
{
  storage.emplace_back(std::move(samples));
}

template<typename T>
std::vector<T> Sequence<T>::pop()
{
  if(storage.empty()) {
    return {};
  }

  auto ret = std::move(storage.front());
  storage.pop_front();
  return ret;
}

template<typename T>
std::chrono::milliseconds Sequence<T>::duration() const
{
  assert(metadata.sampleRate > 0);
  return std::chrono::milliseconds((storage.size() * metadata.sampleCount) / (metadata.sampleRate / 1000));
}

template<typename T>
typename Sequence<T>::Samples::reference Sequence<T>::operator[](size_t pos)
{
  auto store = pos / metadata.sampleCount;
  auto sample = pos % metadata.sampleCount;
  return (*std::next(std::begin(storage), store))[sample];
}

template<typename T>
typename Sequence<T>::Samples::const_reference Sequence<T>::operator[](size_t pos) const
{
  auto store = pos / metadata.sampleCount;
  auto sample = pos % metadata.sampleCount;
  return (*std::next(std::begin(storage), store))[sample];
}

template<typename T>
size_t Sequence<T>::size() const
{
  if(storage.empty()) {
    return 0;
  }
  return (storage.size() - 1) * metadata.sampleCount + storage.back().size();
}

template<typename T>
typename Sequence<T>::iterator Sequence<T>::begin()
{
  return Sequence<T>::iterator(const_cast<Sequence<T>*>(this));
}

template<typename T>
typename Sequence<T>::iterator Sequence<T>::end()
{
  return Sequence<T>::iterator();
}

template<typename T>
typename Sequence<T>::const_iterator Sequence<T>::begin() const
{
  return Sequence<T>::const_iterator(const_cast<Sequence<T>*>(this));
}

template<typename T>
typename Sequence<T>::const_iterator Sequence<T>::end() const
{
  return Sequence<T>::const_iterator();
}


template<typename T>
SequenceIterator<T>::SequenceIterator()
  : seq_(nullptr)
{}

template<typename T>
SequenceIterator<T>::SequenceIterator(Sequence<T>* seq)
  : seq_(seq)
  , storage_(std::begin(seq->storage))
{
  if(storage_ != std::end(seq->storage)) {
    sample_ = std::begin(*storage_);
  }
}

template<typename T>
typename SequenceIterator<T>::reference SequenceIterator<T>::operator*() const
{
  return sample_.operator*();
}

template<typename T>
typename SequenceIterator<T>::pointer SequenceIterator<T>::operator->() const
{
  return sample_.operator->();
}

template<typename T>
SequenceIterator<T>& SequenceIterator<T>::operator++()
{
  if(++sample_ == std::end(*storage_)) {
    if(++storage_ != std::end(seq_->storage)) {
      sample_ = std::begin(*storage_);
    } else {
      *this = SequenceIterator();
    }
  }
  return *this;
}

template<typename T>
SequenceIterator<T>& SequenceIterator<T>::operator--()
{
  if(sample_-- == std::begin(*storage_)) {
    if(storage_-- != std::begin(seq_->storage)) {
      sample_ = std::prev(std::end(*storage_));
    } else {
      *this = SequenceIterator();
    }
  }
  return *this;
}

template<typename T>
SequenceIterator<T>& SequenceIterator<T>::operator++(int)
{
  auto tmp = *this;
  ++(*this);
  return tmp;
}

template<typename T>
SequenceIterator<T>& SequenceIterator<T>::operator--(int)
{
  auto tmp = *this;
  --(*this);
  return tmp;
}

template<typename T>
bool SequenceIterator<T>::operator==(const SequenceIterator& other) const
{
  return true &&
      (seq_ == other.seq_) &&
      (storage_ == other.storage_) &&
      (sample_ == other.sample_);
}

template<typename T>
bool SequenceIterator<T>::operator!=(const SequenceIterator& other) const
{
  return false ||
      (seq_ != other.seq_) ||
      (storage_ != other.storage_) ||
      (sample_ != other.sample_);
}

} // namespace audio

#endif // AUDIO_SEQUENCE_IMPL_H
