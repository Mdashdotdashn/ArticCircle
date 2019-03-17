#pragma once

#include "../dsp.h"
#include "../random.h"

// Basic Shapes
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

// Band limited helpers

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

template <typename T>
T polyBlamp2(const T& phase, const T& phaseInc, const T& discontinuity = T(0.5))
{
  auto result = T(0);

  result += select(
    (phase >= discontinuity) & (phase < (discontinuity + phaseInc)),
    -cube((phase - discontinuity) / phaseInc - T(1)), T(0));

  result += select(
    (phase >= (discontinuity - phaseInc)) & (phase < discontinuity),
    cube((phase - discontinuity) / phaseInc + T(1)), T(0));

  const auto normalisationScaling = T(6);
  return result / normalisationScaling;
}

//! The same as polyBlamp2 but with a fixed discontinuity position at phase = 0|1
template <typename T>
T polyBlamp2Fixed(const T& phase, const T& phaseInc)
{
  auto result = T(0);

  result += select(phase < phaseInc, -cube(phase / phaseInc - T(1)), T(0));
  result += select(phase > T(1) - phaseInc, cube((phase - T(1)) / phaseInc + T(1)), T(0));

  const auto normalisationScaling = T(6);
  return result / normalisationScaling;
}

// Band limited Shapes

template <typename T>
T rectPolyBlep(const T& phase, const T& phaseIncrement, const T& midPoint = T(0.5))
{
  //  The BLEP residual is scaled by the height of the step.
  const auto stepScaling = T(2);

  //  Discontinuities occur at 0 and midPoint.
  return rect(phase, midPoint) + stepScaling * polyBlep1Fixed(phase, phaseIncrement)
         - stepScaling * polyBlep1(phase, phaseIncrement, midPoint);
}

template <typename T>
T sawPolyBlep(const T& phase, const T& phaseIncrement)
{
  // The BLEP residual is scaled by the height of the step
  const auto stepScaling = T(2);
  return saw(phase) - stepScaling * polyBlep1(phase, phaseIncrement, T(0.5));
}

template <typename T>
T trianglePolyBlamp(const T& phase, const T& phaseIncrement)
{
  const auto slopeScaling = phaseIncrement * T(8);

  //  Discontinuities occur at 0.25 and 0.75
  return triangle(phase) - slopeScaling * polyBlamp2(phase, phaseIncrement, T(0.25))
         + slopeScaling * polyBlamp2(phase, phaseIncrement, T(0.75));
}

// Random

template <typename T>
T rand();

template <>
float rand<float>()
{
  const auto val = random::uniform_distribution();
  return ((val & 0x3FFFFFFF) >> 15) / 32767.f;
}

template <>
double rand<double>()
{
  const auto val = random::uniform_distribution();
  return ((val & 0x3FFFFFFF) >> 15) / 32767.0;
}

template <>
sample_t rand<sample_t>()
{
  const auto val = random::uniform_distribution();
  return sample_t::fromRatio((val & 0x3FFFFFFF) >> 15, 32767);
}
