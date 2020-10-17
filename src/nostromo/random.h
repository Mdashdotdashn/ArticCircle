#pragma once

#include <chrono>
#include <cmath>
#include <time.h>

// ported from https://github.com/MKlimenko/random/

struct random {
	///<summary>Get runtime seed from the <c>std::chrono::high_resolution_clock</c><summary>
	///<returns>Seed in the range of [0, std::uint32_t_max]</returns>
	static auto get_seed() {
		return static_cast<std::uint32_t>(micros());
	}

	///<summary>Real uniform distribution</summary>
	///<returns>Uniform distributed value in the range of [0, 1]</returns>
	static uint32_t uniform_distribution(std::uint32_t init = 0) {
		static std::uint32_t val = 0x12345;
		if (init) {
			val = init;
			return 0;
		}
		val = val * 214013 + 2531011;
    return val;
	}
};

// Random

template <typename T>
T rand();

template <>
float rand<float>()
{
  const auto val = random::uniform_distribution();
  return ((val & 0x3FFFFFFF) >> 15) / 32767.f;
}

template <>
double rand<double>()
{
  const auto val = random::uniform_distribution();
  return ((val & 0x3FFFFFFF) >> 15) / 32767.0;
}

template <>
sample_t rand<sample_t>()
{
  const auto val = random::uniform_distribution();
  return sample_t::fromRatio((val & 0x3FFFFFFF) >> 15, 32767);
}
