#include "clock.h"

void PingablePhaser::reset(float samplerate)
{
  samplerate_ = samplerate;
  lastPing_ = 0;
  phase_ = 0;
  phaseIncrease_ = 0;
}

void PingablePhaser::ping(uint32_t pingTime)
{
  if (lastPing_ != 0)
  {
    // Get new tempo from interval
    const auto delta = float(pingTime - lastPing_);
    targetTempo_ = samplerate_ / delta * 60.f;
    // Adapt phaser speed to we catch up phase wise
    const auto offset = sample_t::frac(phase_ + sample_t(0.5)) - sample_t(0.5);
    phaseIncrease_ = sample_t(1.f / delta) * (sample_t(1) - offset);
  }
  lastPing_ = pingTime;
}

sample_t PingablePhaser::tick()
{
  phase_ = sample_t::frac(phase_ + phaseIncrease_);
  return phase_;
}

float PingablePhaser::tempo() const
{
  return targetTempo_;
}

void PhaserDivider::setAmount(int amount)
{
  amount_ = fixed(amount);
  lastValue_ = 0;
}

sample_t PhaserDivider::tick(sample_t value)
{
  return fixed::frac(fixed(value) * amount_);
}
