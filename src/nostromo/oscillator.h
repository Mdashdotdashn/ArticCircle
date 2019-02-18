#pragma once

#include "math.h"
#include <functional>

template <typename T>
class Phasor
{
public:
  Phasor()
  {};

  void reset(const float samplerate)
  {
    phase_ = T(0);
    samplerate_ = samplerate;
    updatePhaseInc();
  }

  void setFrequency(const float frequency)
  {
    frequency_ = frequency;
    updatePhaseInc();
  }

  T tick()
  {
    phase_ = frac(phase_ + phaseIncrease_);
    return phase_;
  }

private:

  void updatePhaseInc()
  {
    phaseIncrease_ = T(frequency_ / samplerate_);
  }

private:
  T phase_;
  T phaseIncrease_;
  float frequency_ = 440.;
  float samplerate_ = kSampleRate;
};

template <typename T>
class Oscillator
{
public:
  using Ticker = std::function<T(const T&)>;

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
    return ticker_ ? ticker_(phase) : T(0);
  }
private:
  Phasor<T> phasor_;
  Ticker ticker_;
};
