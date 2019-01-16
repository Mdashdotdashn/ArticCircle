#pragma once

template <typename T>
T SFastSine(const T& value)
{
  const T xSquared = value * value;

  T currentPower = xSquared * value;
  T result = T(3.138982) * value;
  result -= T(5.133625) * currentPower;
  currentPower *= xSquared;
  result += T(2.428288) * currentPower;
  currentPower *= xSquared;
  result -= T(0.433645) * currentPower;

  return result;
}

template <typename T>
T Sine(const T& in)
{
  return SFastSine<T>((in - T(0.5)) * T(2));
}
