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

#ifndef incl_HPHP_JIT_VASM_LOWER_H_
#define incl_HPHP_JIT_VASM_LOWER_H_

#include "hphp/runtime/vm/jit/vasm.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct Vunit;

/*
 * Vasm-to-vasm lowering state.
 */
struct VLS {
  Vunit& unit;
  int vreg_restrict_level;

  bool allow_vreg() const {
    return vreg_restrict_level >= 0;
  }
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Toplevel vasm-to-vasm lowering pass.
 *
 * The `lower_impl' callback should have the signature:
 *
 *    void lower_impl(const VLS& env, Vinstr& inst, Vlabel b, size_t i);
 *
 * where `b' and `i' are the block and code index of `inst' in `unit'.  This
 * callback is responsible for any architecture-specific lowering that is
 * needed for `inst'.
 */
template<class Vlower>
void vasm_lower(Vunit& unit, Vlower lower_impl);

/*
 * Lower a single instruction.
 *
 * Replaces the instruction at `unit.blocks[b].code[i]` with an appropriate
 * sequence of one or more instructions.
 */
void vlower(VLS& env, Vlabel b, size_t i);

///////////////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/vm/jit/vasm-lower-inl.h"

#endif
