#pragma once

//------------------------------------------------------------------------------

template <typename T>
T Sine(const T& in)
{
  const auto FastSine = [](const T& value) -> T
  {
    const T xSquared = value * value;

    T currentPower = xSquared * value;
    T result = T(3.138982) * value;
    result -= T(5.133625) * currentPower;
    currentPower *= xSquared;
    result += T(2.428288) * currentPower;
    currentPower *= xSquared;
    result -= T(0.433645) * currentPower;

    return result;
  };

  return FastSine((in - T(0.5)) * T(2));
}

template <typename T>
class ExponentialSegment
{
public:

  void init(const T& value)
  {
    from_ = value;
    to_ = value;
    ramp_ = T(0);
    eos_ = true;
    speed_ = 0;

    current_ = T(0);
  }

  void ramp(const T& from, const T& to, const uint32_t timeInTriggerStep)
  {
    from_= from;
    to_ = to;

    eos_ = (timeInTriggerStep == 0)  ;

    if (!eos_)
    {
      speed_ = T(powf(kNoiseFloor, 1.f/float(timeInTriggerStep)));
    }

    ramp_ = eos_ ? T(0) : T(1); // Ramp goes from 1.0 -> 0 (kNoiseFloor)
    current_ = eos_ ? to_ : from_;
  }


  T tick()
  {
    ramp_ *= speed_;
    current_ = lerp(to_, from_, ramp_);
    eos_ = (ramp_ <= T(kNoiseFloor));
    return current_;
  }

  T value()
  {
    return current_;
  }

  bool eos()
  {
    return eos_;
  }

private:
  constexpr static float kNoiseFloor = 1e-4f;
  T from_;
  T to_;

  bool eos_;

  T speed_;
  T ramp_;

  T current_;
};
