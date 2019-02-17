#pragma once

#include <cstdint>

template <typename T>
class RampedValue
{
public:
  explicit RampedValue(const T& value = T(0))
    : mCurrent(value)
    , mTarget(value)
    , mIncrement(T(0))
    , mTicksToCompletion(0u)
    , mDurationInTicks(0)
  {
  }

  void setValue(const T& value)
  {
    mCurrent = value;
    mTarget = value;
    mTicksToCompletion = 0u;
    mDurationInTicks = 0u;
  }

  void rampTo(const T& target, const std::uint64_t ticksToCompletion)
  {
    mTarget = target;
    mDurationInTicks = ticksToCompletion;

    if (ticksToCompletion <= 1u || mTarget == mCurrent)
    {
      mCurrent = target;
      mTicksToCompletion = 0u;
    }
    else
    {
      mTicksToCompletion = ticksToCompletion - 1u;
      mIncrement = (mTarget - mCurrent) / T(mTicksToCompletion);
    }
  }

  bool isRamping() const { return mTicksToCompletion > 0u; }

  T value() const { return mCurrent; }

  T targetValue() const { return mTarget; }

  T tick()
  {
    const auto result = mCurrent;

    if (mTicksToCompletion > 0u)
    {
      --mTicksToCompletion;
      mCurrent = mTarget - T(mTicksToCompletion) * mIncrement;
    }
    else
    {
      mCurrent = mTarget;
    }

    return result;
  }

  std::uint64_t durationInTicks() const { return mDurationInTicks; }

  std::uint64_t ticksToCompletion() const { return mTicksToCompletion; }

private:
  T mCurrent;
  T mTarget;
  T mIncrement;
  std::uint64_t mTicksToCompletion;
  std::uint64_t mDurationInTicks;
};
