#pragma once

#include <chrono>
#include <cmath>
#include <time.h>

// ported from https://github.com/MKlimenko/random/

template <typename T>
class Random {

public:
	Random()
	{
		seed();
	}

	uint32_t seed(std::uint32_t init = 0) {
		if (init)
		{
			current_ = init;
		}
		else
		{
			current_ = static_cast<std::uint32_t>(micros());
		}
		return current_;
	}

	T tick();

private:
	uint32_t tickInternal()
	{
		current_ = current_ * 214013 + 2531011;
		return current_;
	}
	std::uint32_t current_ = 0x12345;
};

template <>
uint32_t Random<uint32_t>::tick()
{
	return tickInternal();
}

template <>
float Random<float>::tick()
{
	return ((tickInternal() & 0x3FFFFFFF) >> 15) / 32767.f;
}

template <>
sample_t Random<sample_t>::tick()
{
	return sample_t::fromRatio((tickInternal() & 0x3FFFFFFF) >> 15, 32767);
}
