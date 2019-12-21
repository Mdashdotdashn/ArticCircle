#pragma once

#include "math.h"
#include "fixed.h"
#include <cmath>

template <typename T>
T select(bool condition, const T& a, const T&b)
{
  return condition ? a : b;
}

//------------------------------------------------------------------------------

template <typename T>
T Sine(const T& x)
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

  return FastSine((frac(x) - T(0.5)) * T(2));
}

template <typename T>
T Cosine(const T& x)
{
  return Sine(x + T(0.25));
}

template <typename T>
T quadraticSine(const T& x)
{
  const auto evaluatorFn = [](const T& a)
  {
    return -a * a +T(0.75) + a;
  };

  if (x < T(0.5))
  {
    return evaluatorFn(
      x < T(0.25)
      ? (x - T(0.25 / 2.)) * T(4)
      : (x - T(0.25 * 3. / 2.)) * T(-4)
    );
  }
  else
  {
    return -evaluatorFn(
      x < T(0.75)
      ? (x - T(0.5 + 0.25 / 2.)) * T(4)
      : (x - T(0.5 + 0.25 * 3. / 2.)) * T(-4)
    );
  }
}

template <typename T>
T quadraticCosine(const T& x)
{
  return quadraticSine(frac(x + T(0.25)));
}

template <typename T>
std::pair<T, T> quadraticSinCos(const T& x)
{
  const auto evaluatorFn = [](const T& a)
  {
    const auto c = T(sqrt(2.) / 2.);
    return (T(2) - T(4) * c) *a *a + c;
  };

  if (x < T(0.5))
  {
    if (x < T(0.25))
    {
      const auto a = (x - T(0.25 / 2.)) * T(4);
      const auto t = evaluatorFn(a);
      return std::make_pair<T, T>(t + a, t - a);
    }
    else
    {
      const auto a = (x - T(0.25 * 3. / 2.)) * T(-4);
      const auto t = evaluatorFn(a);
      return std::make_pair<T, T>(t + a, a - t);
    }
  }
  else
  {
    if (x < T(0.75))
    {
      const auto a = (x - T(0.5 + 0.25 / 2.)) * T(4);
      const auto t = evaluatorFn(a);
      return std::make_pair<T, T>(-t - a, a - t);
    }
    else
    {
      const auto a = (x - T(0.5 + 0.25 * 3. / 2.)) * T(-4);
      const auto t = evaluatorFn(a);
      return std::make_pair<T, T>(- t - a, t - a);
    }
  }
}

//------------------------------------------------------------------------------

constexpr static float calcSlewCoeff(uint32_t sampleCount, float noiseFloor = 1e-4f)
{
  return exp(log(noiseFloor)/float(sampleCount));
}

static inline float onePoleCoeff(const float samplerate,
  const float timeConstantInSeconds)
{
  const float timeConstantInSamples = timeConstantInSeconds * samplerate;
  if (timeConstantInSamples > 0.f)
  {
    return 1.f - expf(-1.f / timeConstantInSamples);
  }
  return 1.f;
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

  void reset()
  {
    slew_.init(T(0));
    target_ = T(0);
  }

  void setSlopes(const uint32_t attack, const uint32_t decay)
  {
    setCoeffs(calcSlewCoeff(attack), calcSlewCoeff(decay));
  }

  void setCoeffs(const T& a, const T& d)
  {
    slew_.setCoefficients(a, d);
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

template <typename T>
class LinearADEnvelope
{
  constexpr static float kNoiseFloor = 1e-4;

public:
  void init()
  {
    attackCoef_ = releaseCoef_ = T(0.1);
    target_ = T(0);
    value_ = T(0);
  }

  void reset()
  {
    value_ = (0);
    target_ = T(0);
  }

  void setSlopes(const uint32_t attackInSamples, const uint32_t releaseInSamples)
  {
    attackCoef_ = T(1.f/float(attackInSamples));
    releaseCoef_ = T(1.f/float(releaseInSamples));
  }

  T tick(bool gate)
  {
    if (target_ == T(1))
    {
      value_ += attackCoef_;

      if (value_ >= T(1) && gate)
      {
        value_ = T(1);
      }
      else
      {
        target_ = T(0);
        const auto overshoot = value_ - T(1);
        value_ -= overshoot * (releaseCoef_ - attackCoef_);
      }
    }
    else
    {
      if (value_ > T(kNoiseFloor))
      {
        value_ -= releaseCoef_;
      }
      if (value_ < T(kNoiseFloor))
      {
        value_ = T(0);
      }
    }

    if (gate)
    {
      target_ = T(1);
    }

    return value_;
  }

  T value()
  {
    return value_;
  }

private:
  T target_;
  T value_;
  T attackCoef_;
  T releaseCoef_;
};
