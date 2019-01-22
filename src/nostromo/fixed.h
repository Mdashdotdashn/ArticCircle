#pragma once

#include <math.h>
#include <limits.h>
#include <assert.h>
#include <ostream>
#include <stdint.h>

//------------------------------

#define FP_MAX_VAL INT_MAX
#define FP_MIN_VAL INT_MIN

//------------------------------

// C is the storage container (int32_t for a 32bit for example)

template <typename C,uint8_t F>
class FixedFP {

  template<typename C2, uint8_t F2> friend class FixedFP;

public:

  // Constructors

  inline FixedFP() {
    value_ = 0;
  }

  inline FixedFP(const float& f) {
    value_ = static_cast<C>(f * (1 << shift_));
  }

  inline FixedFP(const double& d) {
    value_ = static_cast<C>(d * (1 << shift_));
  }

  inline FixedFP(const int& i) {
    value_ = i << shift_;
  }

  // Factory

  static inline FixedFP fromValue(const C& value)
  {
    FixedFP fp;
    fp.value_ = value;
    return fp;
  }

  static inline FixedFP fromRatio(const C& numerator, const C& denominator)
  {
    FixedFP n,d;
    n.value_ = numerator;
    d.value_ = denominator;
    return n/d;
  }

  // Min - Max

  static FixedFP SMax()
  {
    FixedFP max;
    max.value_ = FP_MAX_VAL;
    return max;
  }

  static FixedFP SMin()
  {
    FixedFP max;
    max.value_ = FP_MIN_VAL;
    return max;
  }

  template <unsigned char F2>
  inline FixedFP(FixedFP<C, F2> const &rhs)
    :value_(rhs.value_)
  {
    int diff = F - F2;
    if (diff > 0) {
      value_ <<= (diff);
    }
    else {
      value_ >>= (-diff);
    }
  }

  inline FixedFP &operator=(const FixedFP &rhs) {
    value_ = rhs.value_;
    return *this;
  }

  template <unsigned char F2>
  inline FixedFP<C, F> &operator=(const FixedFP<C, F2> &rhs) {
    if (rhs.shift_ < shift_) {
      value_ = (rhs.value_) << (shift_ - rhs.shift_);
    }
    else {
      value_ = (rhs.value_) << (rhs.shift_ - shift_);
    }
    return *this;
  }

  inline operator int() const {
    return value_ >> shift_;
  }

  inline operator float() const {
    return float(value_) / (1 << shift_);
  };

  inline operator double() const {
    return double(value_) / (1 << shift_);
  };

  inline FixedFP &operator+=(const FixedFP &rhs) {
    value_ += rhs.value_;
    return *this;
  }

  inline FixedFP &ads(const FixedFP &rhs)
  {
    C sum = value_ + rhs.value_;

    if (sum >= 0)
    {
      if ((value_ < 0) && (rhs.value_ < 0))
      {
        sum = INT_MIN;
      }
    }
    else
    {
      if ((0 < value_) && (0 < rhs.value_))
      {
        sum = INT_MAX;
      }
    }
    value_ = sum;
    return *this;
  }

  inline FixedFP &operator-=(const FixedFP &rhs) {
    value_ -= rhs.value_;
    return *this;
  }

  inline FixedFP &operator*=(const FixedFP &rhs) {

    value_ = static_cast<typename FixedFP<C, F>::template promote_type<C>::type>(value_)*rhs.value_ >> shift_;
    return *this;
  }

  inline FixedFP &operator/=(const FixedFP &rhs) {

    value_ = (static_cast<typename FixedFP<C, F>::template promote_type<C>::type>(value_) << shift_) / rhs.value_;
    return *this;
  }

  inline bool operator==(const FixedFP &rhs) const {
    return value_ == rhs.value_;
  }

  inline bool operator!=(const FixedFP &rhs) const {
    return value_ != rhs.value_;
  }

  inline bool operator>(const FixedFP &rhs) const {
    return value_ > rhs.value_;
  }

  inline bool operator<(const FixedFP &rhs) const {
    return value_ < rhs.value_;
  }

  inline bool operator>=(const FixedFP &rhs) const {
    return value_ >= rhs.value_;
  }

  inline bool operator<=(const FixedFP &rhs) const {
    return value_ <= rhs.value_;
  }

  inline FixedFP operator+(const FixedFP& rhs)
  {
    FixedFP result(*this);
    result += rhs;
    return result;
  }

  inline FixedFP operator-(const FixedFP& rhs)
  {
    FixedFP result(*this);
    result -= rhs;
    return result;
  }

  inline FixedFP operator*(const FixedFP& rhs)
  {
    FixedFP result(*this);
    result *= rhs;
    return result;
  }

  inline FixedFP operator/(const FixedFP& rhs)
  {
    FixedFP result(*this);
    result /= rhs;
    return result;
  }

  inline FixedFP operator-() const
  {
    FixedFP result(*this);
    result.value_ = -result.value_;
    return result;
  }

  inline friend FixedFP operator+(const FixedFP& lhs, const FixedFP& rhs)
  {
    FixedFP result(lhs);
    result += rhs;
    return result;
  }

  inline friend FixedFP operator-(const FixedFP& lhs, const FixedFP& rhs)
  {
    FixedFP result(lhs);
    result -= rhs;
    return result;
  }

  inline friend FixedFP operator*(const FixedFP& lhs, const FixedFP& rhs)
  {
    FixedFP result(lhs);
    result *= rhs;
    return result;
  }

  inline friend FixedFP operator/(const FixedFP& lhs, const FixedFP& rhs)
  {
    FixedFP result(lhs);
    result /= rhs;
    return result;
  }

  static FixedFP floor(const FixedFP &fp) {
    FixedFP ret(fp);
    ret.value_ = (ret.value_&(~fracMask_));
    return ret;
  }

  static FixedFP frac(const FixedFP &fp) {
    FixedFP ret(fp);
    ret.value_ = (ret.value_&(fracMask_ | signMask_));
    return ret;
  }

  friend std::ostream& operator<< (std::ostream& out, const FixedFP& fp)
  {
    out << "fixed[" << fp.value_ << "," << float(fp) << "]";
    return out;
  }

	C value_ ;

	static const uint8_t shift_=F ;
	static const C fracMask_=(1<<F)-1 ;
	static const C signMask_=(1<<(sizeof(C)-1)) ;

	// Promotion mechanism

	template<typename X,typename U=void>
	struct promote_type {

	} ;
	template<typename U>
	struct promote_type<int8_t,U>
	{
		typedef int16_t type;
	};

	template<typename U>
	struct promote_type<uint8_t,U>
	{
		typedef uint16_t type;
	};

	template<typename U>
	struct promote_type<int16_t,U>
	{
		typedef int32_t type;
	};

	template<typename U>
	struct promote_type<uint16_t,U>
	{
		typedef uint32_t type;
	};

	template<typename U>
	struct promote_type<int32_t,U>
	{
		typedef int64_t type;
	};

	template<typename U>
	struct promote_type<uint32_t,U>
	{
		typedef uint64_t type ;
	};
} ;

template <typename C,uint8_t F>
static FixedFP<C,F> abs(const FixedFP<C,F> &fp)
{
  return (fp < FixedFP<C,F>(0)) ? -fp : fp;
}

template <typename C,uint8_t F>
static FixedFP<C,F> max(const FixedFP<C,F> &fp1, const FixedFP<C,F> &fp2)
{
  return (fp1 < fp2) ? fp2 : fp1;
}

template <typename C,uint8_t F>
static FixedFP<C,F> min(const FixedFP<C,F> &fp1, const FixedFP<C,F> &fp2)
{
  return (fp1 < fp2) ? fp1 : fp2;
}

typedef FixedFP<int32_t, 27> sample_t;
