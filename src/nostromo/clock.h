#pragma once

#include "fixed.h"

#include <functional>

class PingablePhaser
{
public:
  void reset(float samplerate);
  void ping(uint32_t pingTime);
  sample_t tick();
  float tempo() const;

private:
  float samplerate_;
  sample_t phase_ = 0;
  sample_t phaseIncrease_ = 0;
  uint32_t lastPing_ = 0;
  float targetTempo_ = 0;
};

class PhaserDivider
{
public:
  using fixed = FixedFP<int32_t, 20>;
  void setAmount(int amount);

  sample_t tick(sample_t value);

private:
  fixed lastValue_;
  fixed amount_;
};

class FlankDetector
{
public:
  bool tick(const sample_t value)
  {
    bool flank = value < last_;
    last_ = value;
    return flank;
  }
private:
  sample_t last_;
};
