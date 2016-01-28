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
#ifndef incl_HPHP_VASM_UTIL_H_
#define incl_HPHP_VASM_UTIL_H_

#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include <algorithm>

namespace HPHP { namespace jit {

struct Vinstr;
struct Vunit;

//////////////////////////////////////////////////////////////////////

/*
 * An instruction that is a "trivial nop" is always removable without changing
 * program behavior.
 */
bool is_trivial_nop(const Vinstr&);

/*
 * Splits any critical edges in `unit'.  Returns true iff the unit was modified.
 */
bool splitCriticalEdges(Vunit& unit);

/*
 * Return a Vreg holding the constant value represented by the given Type.
 */
Vreg make_const(Vunit&, Type);

/*
 * Move all the elements of `in' into `out', replacing `count' elements of
 * `out' starting at `idx'.  `in' is cleared at the end.
 *
 * Example: vector_splice([1, 2, 3, 4, 5], 2, 1, [10, 11, 12]) will change
 * `out' to [1, 2, 10, 11, 12, 4, 5].
 */
template<typename V>
void vector_splice(V& out, size_t idx, size_t count, V& in) {
  auto out_size = out.size();

  if (in.size() > count) {
    // Start by making room in out for the new elements.
    out.resize(out.size() + in.size() - count);

    // Move everything after the to-be-overwritten elements to the new end.
    std::move_backward(out.begin() + idx + count, out.begin() + out_size,
                       out.end());
  } else if (in.size() < count) {
    std::move(out.begin() + idx + count, out.end(),
              out.begin() + idx + in.size());
    out.resize(out.size() + in.size() - count);
  }
  // Move the new elements in.
  std::move(in.begin(), in.end(), out.begin() + idx);
  in.clear();
}

//////////////////////////////////////////////////////////////////////

}}

#endif
