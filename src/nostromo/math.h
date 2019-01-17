#pragma once

template <typename T>
T lerp(const T& a, const T&b, const T& mix)
{
  return a + (b - a) * mix;
}
