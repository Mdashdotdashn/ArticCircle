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

//------------------------------------------------------------------------------

static float calcSlewCoeff(uint32_t sampleCount, float noiseFloor = 1e-4f)
{
  return exp(log(noiseFloor)/float(sampleCount));
}

//------------------------------------------------------------------------------

template <typename T>
class Slew
{
public:

  void init(const T& value)
  {
    value_ = value;
  }

  void setCoefficients(const T&up, const T& down)
  {
    up_ = up;
    down_ = down;
  }

  T tick(const T& target)
  {
      const auto coeff = target > value_ ? up_ : down_;
      value_ = target + (value_ - target) * coeff;
      return value_;
  }

  T value() const
  {
    return value_;
  }

private:
  T value_;
  T up_;
  T down_;
};

//------------------------------------------------------------------------------

template <typename T>
class ADEnvelope
{
public:
  constexpr static float kNoiseFloor = 1e-4;

  void init()
  {
    slew_.init(T(0));
    target_ = T(0);
  }

  void setCoefficients(const T& attack, const T& decay)
  {
    slew_.setCoefficients(attack, decay);
  }

  T tick(bool gate)
  {
    if (target_ == T(1))
    {
      if (target_ - slew_.value() < T(kNoiseFloor))
      {
        target_ = T(0);
      }
    }

    if (gate)
    {
      target_ = T(1);
    }

    return slew_.tick(target_);
  }

  T value()
  {
    return slew_.value();
  }

private:
  Slew<T> slew_;
  T target_;
};
