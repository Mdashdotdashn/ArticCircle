#pragma once

template <int SIZE, int TickToDecay = 8>
class TriggerSizer
{
public:
  TriggerSizer()
  {
    ticksLeft_ = 0;
  }

  void feed(bool gate)
  {
    if (gate) ticksLeft_ = TickToDecay;
  }

  uint8_t updateSize()
  {
    uint8_t size = (ticksLeft_ * SIZE) / TickToDecay;
    if (ticksLeft_ >0) ticksLeft_--;
    return size;
  }

private:
  uint8_t ticksLeft_;
};
