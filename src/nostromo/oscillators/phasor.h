#pragma once

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

  T phaseInc() const
  {
    return phaseIncrease_;
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
