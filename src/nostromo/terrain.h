#pragma once

namespace terrain
{
  // This is used to defined where we're going to evaluate the Terrain
  // shape. It contains the center and radius of the evaluation circle
  // as well as the scaling necessary to bind the trajectory to [-1,1]

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


//------------------------------------------------------------------------------

  template <typename T>
  class Evaluator
  {
    enum class ProcessingState
    {
      Evaluating,
      Done
    };
    using Geometry_t = Geometry<T>;

    constexpr static std::size_t kSubSampleCount = 10;

  public:
    Evaluator(const perlin::Noise<T>& terrain)
      : terrain_(terrain)
      , processingState_(ProcessingState::Done){};

    void Reset(const Geometry_t& geometry)
    {
      processingState_ = ProcessingState::Evaluating;
      geometry_ = geometry;
      phasor_ = T(0);
      noiseRange_ = std::pair<T, T>(2, -2);
      subSampleCount_ = kSubSampleCount - 1;
    }

    bool Finished() { return processingState_ == ProcessingState::Done; }

    Geometry_t GetGeometry()
    {
      return geometry_;
    }

    void evaluateNextStep()
    {
      const auto increment = T(0.01); // Sample 100 points

      const auto sinCos = quadraticSinCos(phasor_);
      const auto posX = geometry_.x + geometry_.radius * sinCos.first;
      const auto posY = geometry_.y + geometry_.radius * sinCos.second;
      const auto value = terrain_.calc(posX, posY, geometry_.z);

      noiseRange_.first = std::min(noiseRange_.first, value);
      noiseRange_.second = std::max(noiseRange_.second, value);

      phasor_ += increment;
      if (phasor_ > T(1))
      {
        geometry_.scalingOffset = (noiseRange_.second + noiseRange_.first) / T(2);
        geometry_.scalingRatio = (noiseRange_.second - noiseRange_.first) / T(2);
        processingState_ = ProcessingState::Done;
      }
    }

    void Tick()
    {
      if (!Finished())
      {
        subSampleCount_ = (subSampleCount_ + 1) % kSubSampleCount; // We can't afford every sample
        if (subSampleCount_ == 0)
        {
          evaluateNextStep();
        }
      }
    }

  private:
    const perlin::Noise<T>& terrain_;
    ProcessingState processingState_;
    std::pair<T, T> noiseRange_;
    Geometry_t geometry_;
    T phasor_;
    std::size_t subSampleCount_ = 0;
  };

//------------------------------------------------------------------------------

  template <typename T>
  class Renderer
  {
    using Geometry_t = Geometry<T>;

  public:
    Renderer(const perlin::Noise<T>& terrain)
      : terrain_(terrain)
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

      const auto evaluateScaledNoise = [this](const Geometry_t& g, const T& sx, const T& cy) -> T
      {
        const auto posX = g.x + g.radius * sx;
        const auto posY = g.y + g.radius * cy;
        const auto value = this->terrain_.calc(posX, posY, g.z);
        return (value - g.scalingOffset) / g.scalingRatio;
      };

      const auto sx = Sine(phasor);
      const auto sy = Cosine(phasor);

      if (fading_)
      {
        const T interpolation =
          (T(fadeCycles_ - fadeCycleCount_) + phasor) / T(fadeCycles_);

        return lerp(evaluateScaledNoise(fromGeometry_, sx, sy),
                    evaluateScaledNoise(toGeometry_, sx, sy),
                    interpolation);
      }

      return evaluateScaledNoise(fromGeometry_, sx, sy);
    }

  private:
    const perlin::Noise<T>& terrain_;

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

//------------------------------------------------------------------------------

  template <typename T>
  class Oscillator
  {
    using Geometry_t = Geometry<T>;

  public:
    Oscillator()
    : evaluator_(terrain_)
    , renderer_(terrain_)
    {};

    void setParameters(T radius, T x, T y, T z)
    {
      queuedGeometry_ = {x, y, z, radius};
      hasQueuedGeometry_ = true;
    }

    T tick(T phasor)
    {
      const auto evaluating = !evaluator_.Finished();
      evaluator_.Tick();
      const auto evaluationFinished = evaluator_.Finished();
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
    perlin::Noise<T> terrain_;
    Evaluator<T> evaluator_;
    Renderer<T> renderer_;

    Geometry_t queuedGeometry_;
    bool hasQueuedGeometry_ = false;
  };
}
