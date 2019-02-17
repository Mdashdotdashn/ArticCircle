#pragma once

#include <algorithm>

template <typename T>
T lerp(const T& a, const T&b, const T& mix)
{
  return a + (b - a) * mix;
}

template <typename T>
T clamp(const T& a,const T&low, const T&high)
{
  return std::max(std::min(a,high),low);
}

/*
template <typename T>
T frac(const T& x)
{
  return x - floor(x);
}*/
