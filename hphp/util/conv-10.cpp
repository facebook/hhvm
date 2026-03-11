/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/util/conv-10.h"

#include <cstring>

namespace HPHP {

namespace {

// Two-digits-at-a-time lookup table. Each pair of characters represents
// the decimal digits for values 00-99. This halves the number of
// divide-by-base operations in the digit extraction loop, which is the
// primary bottleneck. This technique is used in folly, fmtlib, and
// other high-performance integer formatters.
alignas(64) constexpr char kDigitPairs[200] = {
  '0','0', '0','1', '0','2', '0','3', '0','4',
  '0','5', '0','6', '0','7', '0','8', '0','9',
  '1','0', '1','1', '1','2', '1','3', '1','4',
  '1','5', '1','6', '1','7', '1','8', '1','9',
  '2','0', '2','1', '2','2', '2','3', '2','4',
  '2','5', '2','6', '2','7', '2','8', '2','9',
  '3','0', '3','1', '3','2', '3','3', '3','4',
  '3','5', '3','6', '3','7', '3','8', '3','9',
  '4','0', '4','1', '4','2', '4','3', '4','4',
  '4','5', '4','6', '4','7', '4','8', '4','9',
  '5','0', '5','1', '5','2', '5','3', '5','4',
  '5','5', '5','6', '5','7', '5','8', '5','9',
  '6','0', '6','1', '6','2', '6','3', '6','4',
  '6','5', '6','6', '6','7', '6','8', '6','9',
  '7','0', '7','1', '7','2', '7','3', '7','4',
  '7','5', '7','6', '7','7', '7','8', '7','9',
  '8','0', '8','1', '8','2', '8','3', '8','4',
  '8','5', '8','6', '8','7', '8','8', '8','9',
  '9','0', '9','1', '9','2', '9','3', '9','4',
  '9','5', '9','6', '9','7', '9','8', '9','9',
};

} // namespace

folly::StringPiece conv_10(int64_t num, char* buf_end) {
  auto p = buf_end;
  uint64_t magnitude;

  /*
   * On a 2's complement machine, negating the most negative integer
   * results in a number that cannot be represented as a signed integer.
   * Here is what we do to obtain the number's magnitude:
   *      a. add 1 to the number
   *      b. negate it (becomes positive)
   *      c. convert it to unsigned
   *      d. add 1
   */
  if (num < 0) {
    magnitude = static_cast<uint64_t>(-(num + 1)) + 1;
  } else {
    magnitude = static_cast<uint64_t>(num);
  }

  // Extract two digits at a time using a lookup table, halving the
  // number of divisions compared to the one-digit-at-a-time approach.
  while (magnitude >= 100) {
    auto const q = magnitude / 100;
    auto const r = static_cast<uint32_t>(magnitude % 100);
    magnitude = q;
    p -= 2;
    std::memcpy(p, kDigitPairs + r * 2, 2);
  }

  // Handle the remaining 1 or 2 digits without a loop.
  if (magnitude >= 10) {
    p -= 2;
    std::memcpy(p, kDigitPairs + magnitude * 2, 2);
  } else {
    *--p = static_cast<char>('0' + magnitude);
  }

  if (num < 0) *--p = '-';
  return folly::StringPiece{p, static_cast<size_t>(buf_end - p)};
}

}
