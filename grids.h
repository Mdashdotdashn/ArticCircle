#pragma once

#include <cstdint>

namespace grids
{
  class Channel
  {
  public:

    enum class Selector
    {
      BD,
      SD,
      HH,
      BD_ALT,
      SD_ALT,
      HH_ALT
    };

    Channel();

    void reset();
    void advance();

    uint8_t level(Selector selector, uint16_t x, uint16_t y);

  private:
    uint8_t step_;
  };
}
