#pragma once

#include "math.h"
#include "fixed.h"
#include <cmath>

template <typename T>
T select(bool condition, const T& a, const T&b)
{
  return condition ? a : b;
}

template <typename T>
T saw(const T& in)
{
  return T(2) * select(in < T(0.5), in, in - T(1));
}

template <typename T>
T reverseSaw(const T& in)
{
  return -saw(in);
}

template <typename T>
T rect(const T& in, const T& midPoint = T(0.5))
{
  return select(in < midPoint, T(1), T(-1));
}

template <typename T>
T triangle(const T& in)
{
  using std::abs;
  return T(1) - abs(T(2) - abs(T(4) * in - T(3)));
}

template <typename T>
T polyBlep1(const T& phase, const T& phaseInc, const T& discontinuity = T(0.5))
{
  auto result = T(0);

  result += select(
    (phase >= discontinuity) & (phase < (discontinuity + phaseInc)),
    -square((phase - (discontinuity + phaseInc)) / phaseInc), T(0));

  result += select(
    (phase >= (discontinuity - phaseInc)) & (phase < discontinuity),
    square((phase - (discontinuity - phaseInc)) / phaseInc), T(0));

  const auto normalisationScaling = T(2);
  return result / normalisationScaling;
}

//! The same as polyBlep1 but with a fixed discontinuity position at phase = 0|1
template <typename T>
T polyBlep1Fixed(const T& phase, const T& phaseInc)
{
  auto result = T(0);

  result += select(phase < phaseInc, -square(phase / phaseInc - T(1)), T(0));

  result +=
    select(phase > T(1) - phaseInc, square((phase - (T(1) - phaseInc)) / phaseInc), T(0));

  const auto normalisationScaling = T(2);
  return result / normalisationScaling;
}

/*! A bandlimited pulse wave.
 *
 *  Midpoint accepts values `[0..1]` to set the pulsewidth.  A pulsewidth of 0
 *  will produce no output.
 *
 *  If the phaseInc is larger than the pulsewidth then polyBlep1Fixed will
 *  assert.  This limitation is to avoid checking for the case that the Blep
 *  wraps round the reset from 1 to 0.  For release, if the frequency exceeds
 *  this limit then the oscillator will work but is not anti-aliased.  One way
 *  the client can avoid this is by clamping the pulse width for very high
 *  frequencies: - phaseInc < narrowest pulse width < phaseInc.
 */

template <typename T>
T rectPolyBlep(const T& phase, const T& phaseIncrement, const T& midPoint = T(0.5))
{
  //  The BLEP residual is scaled by the height of the step.
  const auto stepScaling = T(2);

  //  Discontinuities occur at 0 and midPoint.
  return rect(phase, midPoint) + stepScaling * polyBlep1Fixed(phase, phaseIncrement)
         - stepScaling * polyBlep1(phase, phaseIncrement, midPoint);
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

static inline float calcSlewCoeff(uint32_t sampleCount, float noiseFloor = 1e-4f)
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

template <typename T>
class LinearADEnvelope
{
  constexpr static float kNoiseFloor = 1e-4;

public:
  void init()
  {
    attackCoef_ = releaseCoef_ = T(0.1);
    target_ = T(0);
  }

  void setCoefficients(const uint32_t attackInSamples, const uint32_t releaseInSamples)
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
