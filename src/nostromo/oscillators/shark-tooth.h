// Copyright: 2019, Ableton AG, Berlin. All rights reserved.

#pragma once

template <typename T>
T wrapPhase(const T& p)
{
  using std::floor;
  return p - floor(p);
}

/*! A Moog style sharkTooth wave
  *
  * The shark tooth shape is windowed combination of a triangle and a saw
  *
  *          *             *             *
  *         * *           * *           *
  *        *   *         *   *         *
  *       *     *       *     *       *
  *       *       *     *       *     *
  *       *         *   *         *   *
  *       *           * *           * *
  *       *             *             *
  *
  * The shape parameter sets the relative proportion of triangle versus saw. When
  * shape = 0, it is a full a triangle. When shape = 1 it is a full saw.

  * For a given shape value (eta), a slice of a saw of size 'eta' is inserted
  * centered at the bottom top of the triangle. The selected slice of saw
  * corresponds to the botom end of the ramp.
  */

template <typename T>
class SharkToothShape
{
public:
  SharkToothShape() {}

  T tick(const T& phase, const T& /*phaseInc*/, const T& shape)
  {
    // the saw needs to be offseted from the triangle depending
    // on the shape parameter. sawDown is the phase of a saw signal
    // ramping continuously from +1 (phase 0) to -1 (phase 1)
    const auto sawPhase = wrapPhase(phase + T(0.25) - T(0.5) * shape);
    const auto sawDown = (sawPhase - T(0.5)) * T(-2);

    const auto triShape = triangle(phase);

    // The window where we use the saw shape is the last part
    const auto selectSaw = T(1) - sawPhase < shape;
    return selectSaw ? sawDown : triShape;
  }

private:
  T mLastPhase = T(-1);
};
