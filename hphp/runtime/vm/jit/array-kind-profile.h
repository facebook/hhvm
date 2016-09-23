/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_ARRAY_KIND_PROFILE_H_
#define incl_HPHP_JIT_ARRAY_KIND_PROFILE_H_

#include "hphp/runtime/base/array-data.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Target profile for the distribution of a value's observed array kinds.
 *
 * The array kinds currently tracked are Empty, Packed, and Mixed.
 */
struct ArrayKindProfile {
  static const uint32_t kNumProfiledArrayKinds = 4;

  /*
   * Register an observed `kind'.
   */
  void report(ArrayData::ArrayKind kind);

  /*
   * Return the fraction of the profiled arrays that had the given `kind'.
   */
  double fraction(ArrayData::ArrayKind kind) const;

  /*
   * Return the total number of samples profiled so far.
   */
  uint32_t total() const {
    uint32_t sum = 0;
    for (uint32_t i = 0; i < kNumProfiledArrayKinds; ++i) {
      sum += m_count[i];
    }
    return sum;
  }

  /*
   * Combine `l' and `r', summing across the kind counts.
   */
  static void reduce(ArrayKindProfile& l, const ArrayKindProfile& r) {
    for (uint32_t i = 0; i < kNumProfiledArrayKinds; ++i) {
      l.m_count[i] += r.m_count[i];
    }
  }

  std::string toString() const;

private:
  uint32_t m_count[kNumProfiledArrayKinds];
};

///////////////////////////////////////////////////////////////////////////////

}}

#endif
