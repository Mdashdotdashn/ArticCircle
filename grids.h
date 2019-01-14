#pragma once

#include <cstdint>

namespace grids
{
  class Channel
  {
  public:
    Channel();

    void reset();

    uint8_t tick(uint8_t part, uint16_t x, uint16_t y);

  private:
    uint8_t step_;
  };
}
