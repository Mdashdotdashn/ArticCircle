#pragma once

#include "fixed.h"

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
