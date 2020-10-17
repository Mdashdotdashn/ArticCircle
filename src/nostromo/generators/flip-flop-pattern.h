#pragma once

#include "../random.h"
#include <array>
#include <numeric>

template <typename T, size_t capacity>
class StaticVector
{
  using iterator = T*;
public:
  StaticVector()
  {
    clear();
  }

  void clear()
  {
    position_ = 0;
  }

  void push_back(const T& value)
  {
    mStorage[position_++] = value;
  }

  size_t size()
  {
    return position_;
  }

  T* data()
  {
    return mStorage.data();
  }

  const T* data() const
  {
    return mStorage.data();
  }

  T operator[](size_t index) noexcept { return mStorage[index]; }

  T* begin() noexcept { return data(); }
  const T*  begin() const noexcept { return mStorage.data(); }
  T* end() noexcept { return data() + position_; }
  const T* end() const noexcept { return mStorage.data() + position_; }

private:
  size_t position_;
  std::array<T, capacity> mStorage;
};

template <size_t kMaxCapacity>
class FlipFlopPatternWeaver
{
  using Storage = StaticVector<size_t, kMaxCapacity>;

public:
  FlipFlopPatternWeaver()
  {
  }

  float random()
  {
    return rand<float>();
  }

  Storage distributeSteps(size_t sliceCount, size_t totalCount, size_t minimum)
  {
    assert(totalCount <= kMaxCapacity);
    assert(totalCount >= sliceCount);
    auto remaining = totalCount;
    Storage result;

    for (size_t i = 0; i < sliceCount - 1; i++)
    {
      // Since we need at least one step per slice, we can't use them all
      const auto available = remaining - (sliceCount - 1 - i);
      const auto allocated = std::max(size_t(std::floor(available *  random())), minimum);
      remaining-= allocated;
      result.push_back(allocated);
    }
    result.push_back(remaining);
    assert(result.size() == sliceCount);
    assert(std::accumulate(result.begin(), result.end(), 0u) == totalCount);
    return result;
  }

  // Generate a flip flop signal with @param n up/down cycles spread over
  // @param size steps. @param density controls the amount time spend in
  // 'high' states versus low state
  //
  // @returns a vector containing indices where a transision from low -> high
  // or high -> low happens
  template <size_t size>
  Storage generate(size_t n, float density)
  {
    // Count how many steps will be in high state
    auto highStepCount = std::max(size_t(std::floor(size * density)), n);
    auto lowStepCount = size - highStepCount;
    // distrubutes high steps
    auto highSteps = distributeSteps(n, highStepCount, 1);
    // distribute low steps
    auto lowSteps = distributeSteps(n, lowStepCount, 1);
    // Weave'em
    Storage result;

    for (size_t i = 0; i < n; i++)
    {
      result.push_back(highSteps[i]);
      result.push_back(lowSteps[i]);
    }
    return result;
  }
};
