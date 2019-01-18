#pragma once

#include <cstdint>

namespace grids
{
  class Channel
  {
  public:
    Channel();

    void reset();
    void advance();

    uint8_t level(uint8_t part, uint16_t x, uint16_t y);

  private:
    uint8_t step_;
  };
}
