#pragma once

#include <vector>
#include <algorithm>

class ScaleCurve
{
  using Pair = std::pair<int,int>; // cv in -> cv out
public:
  ScaleCurve()
  {
    data_.reserve(200);
  }

  void reset(int min, int max)
  {
    data_.clear();
    data_.push_back({min, 0});
    data_.push_back({max, 0});
    lastInput_ = min;
    lastIndex_ = 0;
  };

  void update(int cvin, int value)
  {
    // cached index ?
    if (cvin == lastInput_)
    {
      data_[lastIndex_].second = value;
      return;
    }

    lastInput_ = cvin;

    // consider boundaries
    if (cvin <= data_[0].first)
    {
      lastIndex_ = 0;
      data_[lastIndex_].second = value;

    }
    size_t endIndex = data_.size() -1;
    if (cvin >= data_[endIndex].first)
    {
      lastIndex_ = endIndex;
      data_[lastIndex_].second = value;
    }

    // Find the last value equal or lower
    auto lowIt = data_.begin();
    for (auto it = lowIt; it != data_.end(); it++)
    {
      if (it->first >= cvin)
      {
        break;
      }
      lowIt = it;
    }

    if (lowIt-> first== cvin)
    {
      lastIndex_ = std::distance(lowIt, data_.begin());
      lowIt->second = value;
    }

    const auto insertPoint = std::next(lowIt);
    data_.insert(insertPoint, {cvin, value});
    lastIndex_ = std::distance(data_.begin(), insertPoint);
  }

  int map(int cvin)
  {
    // consider bouldaries
    if (cvin <= data_[0].first) return data_[0].second;
    size_t lastIndex = data_.size() -1;
    if (cvin >= data_[lastIndex].first) return data_[lastIndex].second;
    // it's in the middle

    auto lowIt = data_.begin();
    for (auto it = std::next(lowIt); it != data_.end(); it++)
    {
      if (it->first > cvin)
      {
        break;
      }
      lowIt = it;
    }

    const auto low = *lowIt++;
    const auto high = *lowIt;
    const auto eta = float(cvin - (low.first))/float(high.first - low.first);
    return low.second + int(eta * (high.second - low.second));
  }
private:
  std::vector<Pair> data_;
  size_t lastIndex_;
  int lastInput_;
};
