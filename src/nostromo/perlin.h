#pragma once

#include <array>
#include <cstddef>
#include <algorithm>
#include <cmath>

#include "fixed.h"
#include "math.h"
#include "ramped_value.h"

// Perlin noise computation, adapted to run on teensy with fixed point.
// Original code from https://github.com/sol-prog/Perlin_Noise

namespace perlin
{
namespace detail
{
// Since some constants of fade require higher value than what sample_t
// can hold, we lower the precision if using sample_t;
template <typename T>
struct selector
{
  using type_t = T;
};

template <>
struct selector<sample_t>
{
  using type_t = FixedFP<int32_t, 15>;
};

template <typename T>
T fade(const T& t)
{
  using Fixed = typename selector<T>::type_t;
  const auto f = Fixed(t);
  return f * f * f * (f * (f * Fixed(6) - Fixed(15)) + Fixed(10));
}

template <typename T>
T grad(std::size_t hash, const T& x, const T& y, const T& z)
{
  const auto h = int(hash & 15);
  // Convert lower 4 bits of hash inot 12 gradient directions
  T u = h < 8 ? x : y, v = h < 4 ? y : h == 12 || h == 14 ? x : z;
  return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}
} // detail

template <typename T>
class Noise
{
public:
Noise()
{
  // Initialize the permutation vector with the reference values
  const std::array<std::size_t, 256> data =
      {151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233, 7,   225,
       140, 36,  103, 30,  69,  142, 8,   99,  37,  240, 21,  10,  23,  190, 6,   148,
       247, 120, 234, 75,  0,   26,  197, 62,  94,  252, 219, 203, 117, 35,  11,  32,
       57,  177, 33,  88,  237, 149, 56,  87,  174, 20,  125, 136, 171, 168, 68,  175,
       74,  165, 71,  134, 139, 48,  27,  166, 77,  146, 158, 231, 83,  111, 229, 122,
       60,  211, 133, 230, 220, 105, 92,  41,  55,  46,  245, 40,  244, 102, 143, 54,
       65,  25,  63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,  18,  169,
       200, 196, 135, 130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186, 3,   64,
       52,  217, 226, 250, 124, 123, 5,   202, 38,  147, 118, 126, 255, 82,  85,  212,
       207, 206, 59,  227, 47,  16,  58,  17,  182, 189, 28,  42,  223, 183, 170, 213,
       119, 248, 152, 2,   44,  154, 163, 70,  221, 153, 101, 155, 167, 43,  172, 9,
       129, 22,  39,  253, 19,  98,  108, 110, 79,  113, 224, 232, 178, 185, 112, 104,
       218, 246, 97,  228, 251, 34,  242, 193, 238, 210, 144, 12,  191, 179, 162, 241,
       81,  51,  145, 235, 249, 14,  239, 107, 49,  192, 214, 31,  181, 199, 106, 157,
       184, 84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236, 205, 93,
       222, 114, 67,  29,  24,  72,  243, 141, 128, 195, 78,  66,  215, 61,  156, 180};

  // Creat a duplicated  permutation vector
  auto iterator = std::begin(p);
  for (auto i: {1,2})
  {
    iterator = std::copy(std::begin(data), std::end(data), iterator);
  }
}

T calc(const T& xc, const T& yc, const T& zc) const
{
  using namespace detail;

  // Find the unit cube that contains the point
  const size_t pX = size_t(int(floor(xc))) & 255;
  const size_t pY = size_t(int(floor(yc))) & 255;
  const size_t pZ = size_t(int(floor(zc))) & 255;

  // Find relative x, y,z of point in cube
  const auto x = xc - floor(xc);
  const auto y = yc - floor(yc);
  const auto z = zc - floor(zc);

  // Compute fade curves for each of x, y, z
  const auto u = fade(x);
  const auto v = fade(y);
  const auto w = fade(z);

  // Hash coordinates of the 8 cube corners
  const auto A = p[pX] +pY;
  const auto AA = p[A] + pZ;
  const auto AB = p[A + 1] + pZ;
  const auto B = p[pX + 1] + pY;
  const auto BA = p[B] + pZ;
  const auto BB = p[B + 1] + pZ;

  const auto ilerp = [](const T& mix, const T& a, const T& b)
  {
    return lerp(a, b, mix);
  };

  // Add blended results from 8 corners of cube
  const auto res = ilerp(
    w,
    ilerp(v, ilerp(u, grad(p[AA], x, y, z), grad(p[BA], x - T(1), y, z)),
         ilerp(u, grad(p[AB], x, y - T(1), z), grad(p[BB], x - T(1), y - T(1), z))),
    ilerp(
      v, ilerp(u, grad(p[AA + 1], x, y, z - T(1)), grad(p[BA + 1], x - T(1), y, z - T(1))),
      ilerp(u, grad(p[AB + 1], x, y - T(1), z - T(1)), grad(p[BB + 1], x - T(1), y - T(1), z - T(1)))));
  return (res + T(1)) / T(2);
}

private:
  // The permutation vector
  std::array<std::size_t, 256 * 2> p;
};

template <typename T>
struct Geometry
{
  Geometry(const T& x = T(0),
           const T& y = T(0),
           const T& z = T(0),
           const T& radius = T(0.2))
    : x(x)
    , y(y)
    , z(z)
    , radius(radius)
    , scalingOffset(T(0))
    , scalingRatio(T(1)){};


  T x;
  T y;
  T z;

  T radius;

  T scalingOffset;
  T scalingRatio;
};

template <typename T>
T SEvaluateNoise(const Noise<T>& noiseSource,
                 const T& x,
                 const T& y,
                 const T& z,
                 const T& radius,
                 const T& phasor)
{
  T posX = x + radius * Sine(phasor);
  T posY = y + radius * Cosine(phasor);
  return noiseSource.calc(posX, posY, z);
}

template <typename T>
T SEvaluateScaledNoise(const Noise<T>& noiseSource,
                       const Geometry<T>& geometry,
                       const T& phasor)
{
  T rawValue = SEvaluateNoise(
    noiseSource, geometry.x, geometry.y, geometry.z, geometry.radius, phasor);
  return (rawValue - geometry.scalingOffset) * geometry.scalingRatio;
}

template <typename T>
class Evaluator
{
  enum class ProcessingState
  {
    Evaluating,
    Done
  };
  using Geometry_t = Geometry<T>;

public:
  Evaluator(const Noise<T>& noiseSource)
    : noiseSource_(noiseSource)
    , processingState_(ProcessingState::Done){};

  void Reset(const Geometry_t& geometry)
  {
    processingState_ = ProcessingState::Evaluating;
    geometry_ = geometry;
    phasor_ = T(0);
    noiseRange_ = std::pair<T, T>(2, -2);
  }

  bool Finished() { return processingState_ == ProcessingState::Done; }

  Geometry_t GetGeometry()
  {
    assert(Finished());
    return geometry_;
  }

  void Tick(const T& increment)
  {
    T value = SEvaluateNoise(
      noiseSource_, geometry_.x, geometry_.y, geometry_.z, geometry_.radius, phasor_);
    noiseRange_.first = std::min(noiseRange_.first, value);
    noiseRange_.second = std::max(noiseRange_.second, value);

    phasor_ += increment;
    if (phasor_ > T(1))
    {
      geometry_.scalingOffset = (noiseRange_.second + noiseRange_.first) / T(2);
      geometry_.scalingRatio = T(2) / (noiseRange_.second - noiseRange_.first);
      processingState_ = ProcessingState::Done;
    }
  }

private:
  const Noise<T>& noiseSource_;
  ProcessingState processingState_;
  std::pair<T, T> noiseRange_;
  Geometry_t geometry_;
  T phasor_;
};

template <typename T>
class Renderer
{
  using Geometry_t = Geometry<T>;

public:
  Renderer(const Noise<T>& noiseSource)
    : noiseSource_(noiseSource)
    , fading_(false)
    , fadeCycleCount_(0)
    , fadeCycles_(1)
    , hasQueuedGeometry_(false)
  {
  }

  void QueueGeometry(const Geometry_t& geometry)
  {
    queuedGeometry_ = geometry;
    hasQueuedGeometry_ = true;
  }

  void SetFadeCycles(const int fadeCycles) { fadeCycles_ = fadeCycles; }

  T Tick(const T& phasor)
  {
    // If we were fading and wrapping, we swap to -> from

    if (phasor < lastPhasor_)
    {
      if (fading_)
      {
        if (--fadeCycleCount_ == 0)
        {
          fromGeometry_ = toGeometry_;
          fading_ = false;
        }
      }

      // If we're not fading and have a queued geometry, trigger fading

      if (!fading_ && hasQueuedGeometry_)
      {
        toGeometry_ = queuedGeometry_;
        fading_ = true;
        fadeCycleCount_ = fadeCycles_;
        hasQueuedGeometry_ = false;
      }
    }

    lastPhasor_ = phasor;

    if (fading_)
    {
      const T interpolation =
        (T(fadeCycles_ - fadeCycleCount_) + phasor) / T(fadeCycles_);
      return lerp(SEvaluateScaledNoise(noiseSource_, fromGeometry_, phasor),
                  SEvaluateScaledNoise(noiseSource_, toGeometry_, phasor), interpolation);
    }

    return SEvaluateScaledNoise(noiseSource_, fromGeometry_, phasor);
  }

private:
  const Noise<T>& noiseSource_;

  RampedValue<T> mixRamp_;

  Geometry_t fromGeometry_;
  Geometry_t toGeometry_;

  bool fading_;
  int fadeCycleCount_;
  int fadeCycles_;
  T lastPhasor_;

  bool hasQueuedGeometry_;
  Geometry_t queuedGeometry_;
};

template <typename T>
class Oscillator
{
  using Geometry_t = perlin::Geometry<T>;

public:
  Oscillator()
  : evaluator_(noiseSource_)
  , renderer_(noiseSource_)
  {};

  void setParameters(T radius, T x, T y, T z)
  {
    queuedGeometry_ = {x, y, z, radius};
    hasQueuedGeometry_ = true;
  }

  void setFadeCycles(const int fadeCycles) { /*renderer_.SetFadeCycles(fadeCycles);*/ }

  T tick(T phasor)
  {
    const bool evaluating = !evaluator_.Finished();
    if (evaluating)
    {
      evaluator_.Tick(T(0.01));
    }
    const bool evaluationFinished = evaluator_.Finished();
    if (evaluating && evaluationFinished)
    {
      renderer_.QueueGeometry(evaluator_.GetGeometry());
    }
    if (evaluationFinished && hasQueuedGeometry_)
    {
      evaluator_.Reset(queuedGeometry_);
      hasQueuedGeometry_ = false;
    }
    return renderer_.Tick(phasor);
  };

private:
  perlin::Noise<T> noiseSource_;
  perlin::Evaluator<T> evaluator_;
  perlin::Renderer<T> renderer_;

  Geometry_t queuedGeometry_;
  bool hasQueuedGeometry_ = false;
};

} // namespace perlin
