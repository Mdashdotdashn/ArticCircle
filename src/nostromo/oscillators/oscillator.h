#pragma once

#include "phasor.h"

#include "../math.h"
#include <functional>

template <typename T>
class Oscillator
{
public:
  using Ticker = std::function<T(const T&, const T&)>;

  void reset(const float samplerate)
  {
    phasor_.reset(samplerate);
  }

  void setFrequency(const float frequency)
  {
    phasor_.setFrequency(frequency);
  }

  void setTicker(const Ticker& ticker)
  {
    ticker_ = ticker;
  }

  T tick()
  {
    const auto phase = phasor_.tick();
    return ticker_ ? ticker_(phase, phasor_.phaseInc()) : T(0);
  }
private:
  Phasor<T> phasor_;
  Ticker ticker_;
};
