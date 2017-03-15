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

namespace HPHP {

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

  /*
   * We use a do-while loop so that we write at least 1 digit
   */
  do {
    auto const q = magnitude / 10;
    auto const r = static_cast<uint32_t>(magnitude % 10);
    *--p = r + '0';
    magnitude = q;
  } while (magnitude);

  if (num < 0) *--p = '-';
  return folly::StringPiece{p, static_cast<size_t>(buf_end - p)};
}

}
