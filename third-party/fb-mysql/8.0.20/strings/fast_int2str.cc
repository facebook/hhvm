/*
MIT License

Copyright (c) 2017 James Edward Anhalt III (jeaiii)
https://github.com/jeaiii/itoa

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stddef.h>
#include <stdint.h>
#include "my_dbug.h"

struct pair {
  char c1, c2;
};

static const pair s_100s[100] = {
    {'0', '0'}, {'0', '1'}, {'0', '2'}, {'0', '3'}, {'0', '4'}, {'0', '5'},
    {'0', '6'}, {'0', '7'}, {'0', '8'}, {'0', '9'}, {'1', '0'}, {'1', '1'},
    {'1', '2'}, {'1', '3'}, {'1', '4'}, {'1', '5'}, {'1', '6'}, {'1', '7'},
    {'1', '8'}, {'1', '9'}, {'2', '0'}, {'2', '1'}, {'2', '2'}, {'2', '3'},
    {'2', '4'}, {'2', '5'}, {'2', '6'}, {'2', '7'}, {'2', '8'}, {'2', '9'},
    {'3', '0'}, {'3', '1'}, {'3', '2'}, {'3', '3'}, {'3', '4'}, {'3', '5'},
    {'3', '6'}, {'3', '7'}, {'3', '8'}, {'3', '9'}, {'4', '0'}, {'4', '1'},
    {'4', '2'}, {'4', '3'}, {'4', '4'}, {'4', '5'}, {'4', '6'}, {'4', '7'},
    {'4', '8'}, {'4', '9'}, {'5', '0'}, {'5', '1'}, {'5', '2'}, {'5', '3'},
    {'5', '4'}, {'5', '5'}, {'5', '6'}, {'5', '7'}, {'5', '8'}, {'5', '9'},
    {'6', '0'}, {'6', '1'}, {'6', '2'}, {'6', '3'}, {'6', '4'}, {'6', '5'},
    {'6', '6'}, {'6', '7'}, {'6', '8'}, {'6', '9'}, {'7', '0'}, {'7', '1'},
    {'7', '2'}, {'7', '3'}, {'7', '4'}, {'7', '5'}, {'7', '6'}, {'7', '7'},
    {'7', '8'}, {'7', '9'}, {'8', '0'}, {'8', '1'}, {'8', '2'}, {'8', '3'},
    {'8', '4'}, {'8', '5'}, {'8', '6'}, {'8', '7'}, {'8', '8'}, {'8', '9'},
    {'9', '0'}, {'9', '1'}, {'9', '2'}, {'9', '3'}, {'9', '4'}, {'9', '5'},
    {'9', '6'}, {'9', '7'}, {'9', '8'}, {'9', '9'}};

/*
 In naive implementation we divide number by 10 in a loop and print
 digit by digit. The problem with that approach is that division operation
 is quite expensive for a CPU.
 Here we convert number into Q32.32 fixed point number format
 ( https://en.wikipedia.org/wiki/Q_(number_format) )
 and divide by 10^(number of digits - 2).
 For example:
 123456789 becomes 12.3456789
 1) Then we print integer part of a number 12
 2) Then we remove integer part of number: 0.3456789
 3) Then we multiply by 100: 34.56789
 Repeat from step 1.
 In order to unroll all loops and avoid division by 10^(number of digits - 2)
 we figure number of digits in the number in advance.
 */

constexpr uint64_t power10(unsigned n) { return n ? 10 * power10(n - 1) : 1; }

template <unsigned N>
uint64_t binary_to_fixed_point(uint32_t u) {
  const uint64_t k = power10(N);
  // make 1. fixed point constant in format 1.N where 1 in
  // upper 32 bits and N zeroes in lower 32 bits
  // N / 5 * N * 53 / 16 -> number of bits in N decimal digits number
  uint64_t t = (1ULL << (32 + N / 5 * N * 53 / 16)) / k + 1 - N / 9;
  // convert number u into 1.N fixed point format
  // for example 12345678 becomes 12345678.000000
  t *= u;
  // shift left N decimal digits
  // 12345678.000000 becomes 12.345678
  t >>= N / 5 * N * 53 / 16;
  t += N / 5 * 4;
  return t;
}
template <unsigned N>
inline void print_two_digits(char *b, uint64_t t) {
  *(pair *)&b[N] = s_100s[t >> 32];
}
template <unsigned N>
inline void shift_print_two_digits(char *b, uint64_t &t) {
  t = 100ULL * uint32_t(t);
  print_two_digits<N>(b, t);
}
template <unsigned N>
inline void print_last_one_digit(char *b, uint64_t t) {
  b[N] = char((10ULL * uint32_t(t)) >> 32) + '0';
}
template <unsigned N>
inline void print_n_digit_number(uint32_t, char *) {
  static_assert(N == 0 || N > 10, "incorrect template specialization");
}
template <>
inline void print_n_digit_number<1>(uint32_t u, char *b) {
  b[0] = char(u) + '0';
}
template <>
inline void print_n_digit_number<2>(uint32_t u, char *b) {
  *(pair *)&b[0] = s_100s[u];
}
template <>
inline void print_n_digit_number<3>(uint32_t u, char *b) {
  uint64_t t = binary_to_fixed_point<1>(u);
  print_two_digits<0>(b, t);
  print_last_one_digit<2>(b, t);
}
template <>
inline void print_n_digit_number<4>(uint32_t u, char *b) {
  uint64_t t = binary_to_fixed_point<2>(u);
  print_two_digits<0>(b, t);
  shift_print_two_digits<2>(b, t);
}
template <>
inline void print_n_digit_number<5>(uint32_t u, char *b) {
  uint64_t t = binary_to_fixed_point<3>(u);
  print_two_digits<0>(b, t);
  shift_print_two_digits<2>(b, t);
  print_last_one_digit<4>(b, t);
}
template <>
inline void print_n_digit_number<6>(uint32_t u, char *b) {
  uint64_t t = binary_to_fixed_point<4>(u);
  print_two_digits<0>(b, t);
  shift_print_two_digits<2>(b, t);
  shift_print_two_digits<4>(b, t);
}
template <>
inline void print_n_digit_number<7>(uint32_t u, char *b) {
  uint64_t t = binary_to_fixed_point<5>(u);
  print_two_digits<0>(b, t);
  shift_print_two_digits<2>(b, t);
  shift_print_two_digits<4>(b, t);
  print_last_one_digit<6>(b, t);
}
template <>
inline void print_n_digit_number<8>(uint32_t u, char *b) {
  uint64_t t = binary_to_fixed_point<6>(u);
  print_two_digits<0>(b, t);
  shift_print_two_digits<2>(b, t);
  shift_print_two_digits<4>(b, t);
  shift_print_two_digits<6>(b, t);
}
template <>
inline void print_n_digit_number<9>(uint32_t u, char *b) {
  uint64_t t = binary_to_fixed_point<7>(u);
  print_two_digits<0>(b, t);
  shift_print_two_digits<2>(b, t);
  shift_print_two_digits<4>(b, t);
  shift_print_two_digits<6>(b, t);
  print_last_one_digit<8>(b, t);
}
template <>
inline void print_n_digit_number<10>(uint32_t u, char *b) {
  uint64_t t = binary_to_fixed_point<8>(u);
  print_two_digits<0>(b, t);
  shift_print_two_digits<2>(b, t);
  shift_print_two_digits<4>(b, t);
  shift_print_two_digits<6>(b, t);
  shift_print_two_digits<8>(b, t);
}
template <unsigned N>
inline void print_n(uint32_t u, char *&b) {
  DBUG_ASSERT(u < power10(N));
  print_n_digit_number<N>(u, b);
  b += N;
}
static inline MY_ATTRIBUTE((always_inline)) void print_number(uint32_t u,
                                                              char *&b) {
  if (u < power10(2)) {
    if (u < power10(1)) {
      print_n<1>(u, b);
    } else {
      print_n<2>(u, b);
    }
  } else if (u < power10(6)) {
    if (u < power10(4)) {
      if (u < power10(3)) {
        print_n<3>(u, b);
      } else {
        print_n<4>(u, b);
      }
    } else {
      if (u < power10(5)) {
        print_n<5>(u, b);
      } else {
        print_n<6>(u, b);
      }
    }
  } else {
    if (u < power10(8)) {
      if (u < power10(7)) {
        print_n<7>(u, b);
      } else {
        print_n<8>(u, b);
      }
    } else {
      if (u < power10(9)) {
        print_n<9>(u, b);
      } else {
        print_n<10>(u, b);
      }
    }
  }
}

extern char *u32toa_jeaiii(uint32_t u, char *b) {
  print_number(u, b);
  b[0] = 0;
  return b;
}

extern char *i32toa_jeaiii(int32_t i, char *b) {
  uint32_t u = i < 0 ? *b++ = '-', 0 - uint32_t(i) : i;
  print_number(u, b);
  b[0] = 0;
  return b;
}

extern char *u64toa_jeaiii(uint64_t n, char *b) {
  uint32_t u;

  if (uint32_t(n >> 32) == 0)
    return u = uint32_t(n), print_number(u, b), b[0] = 0, b;

  uint64_t a = n / 100000000;

  if (uint32_t(a >> 32) == 0) {
    u = uint32_t(a);
    print_number(u, b);
  } else {
    u = uint32_t(a / 100000000);
    print_number(u, b);
    u = a % 100000000;
    print_n<8>(u, b);
  }

  u = n % 100000000;
  print_n<8>(u, b);
  b[0] = 0;
  return b;
}

extern char *i64toa_jeaiii(int64_t i, char *b) {
  uint64_t n = i < 0 ? *b++ = '-', 0 - uint64_t(i) : i;
  return u64toa_jeaiii(n, b);
}

static inline MY_ATTRIBUTE((always_inline)) size_t
    num_digits(const uint32_t u) {
  if (u < power10(2)) {
    return u < power10(1) ? 1 : 2;
  } else if (u < power10(6)) {
    if (u < power10(4)) {
      return u < power10(3) ? 3 : 4;
    } else {
      return u < power10(5) ? 5 : 6;
    }
  } else {
    if (u < power10(8)) {
      return u < power10(7) ? 7 : 8;
    } else {
      return u < power10(9) ? 9 : 10;
    }
  }
}

// print first "ndigits" digits of a number
// in the buffer of buflen length
// if buffer is not big enough, it prints only first buflen digits
static inline MY_ATTRIBUTE((always_inline)) size_t
    print_number_dn(uint32_t u, char *&b, size_t ndigits, const size_t buflen) {
  if (ndigits > buflen) {
    static const uint64_t pow10_64[11] = {
        1,          power10(1), power10(2), power10(3), power10(4), power10(5),
        power10(6), power10(7), power10(8), power10(9), power10(10)};
    const size_t diff = ndigits - buflen;
    const uint32_t p = pow10_64[diff];
    // remove diff trailing digits
    u /= p;
    ndigits = buflen;
  }
  switch (ndigits) {
    case 10:
      print_n<10>(u, b);
      break;
    case 9:
      print_n<9>(u, b);
      break;
    case 8:
      print_n<8>(u, b);
      break;
    case 7:
      print_n<7>(u, b);
      break;
    case 6:
      print_n<6>(u, b);
      break;
    case 5:
      print_n<5>(u, b);
      break;
    case 4:
      print_n<4>(u, b);
      break;
    case 3:
      print_n<3>(u, b);
      break;
    case 2:
      print_n<2>(u, b);
      break;
    case 1:
      print_n<1>(u, b);
      break;
      // case 0: print nothing
  }
  return ndigits;
}

// print number in the buffer of buflen length
// if buffer is not big enough, it prints only first buflen digits
static inline MY_ATTRIBUTE((always_inline)) size_t
    print_number_n(uint32_t u, char *&b, const size_t buflen) {
  size_t digits = num_digits(u);
  return print_number_dn(u, b, digits, buflen);
}

extern size_t u32toa_jeaiii_n(uint32_t u, char *b, const size_t buflen) {
  return print_number_n(u, b, buflen);
}

extern size_t i32toa_jeaiii_n(int32_t i, char *b, const size_t buflen) {
  if (i < 0) {
    if (buflen) {
      *b++ = '-';
      return print_number_n(0 - uint32_t(i), b, buflen - 1) + 1;
    }
    return 0;
  }
  return print_number_n(uint32_t(i), b, buflen);
}

extern size_t u64toa_jeaiii_n(uint64_t n, char *b, const size_t buflen) {
  if (uint32_t(n >> 32) == 0) return print_number_n(uint32_t(n), b, buflen);

  size_t l;
  uint32_t u;
  size_t len = buflen;
  uint64_t a = n / 100000000;

  if (uint32_t(a >> 32) == 0) {
    u = uint32_t(a);
    l = print_number_n(u, b, len);
    len -= l;
  } else {
    u = uint32_t(a / 100000000);
    l = print_number_n(u, b, len);
    len -= l;
    u = a % 100000000;
    l = print_number_dn(u, b, 8, len);
    len -= l;
  }

  u = n % 100000000;
  l = print_number_dn(u, b, 8, len);
  len -= l;
  return buflen - len;
}

extern size_t i64toa_jeaiii_n(int64_t i, char *b, const size_t buflen) {
  if (i < 0) {
    if (buflen) {
      *b++ = '-';
      return u64toa_jeaiii_n(0 - uint64_t(i), b, buflen - 1) + 1;
    }
    return 0;
  }
  return u64toa_jeaiii_n(uint64_t(i), b, buflen);
}
