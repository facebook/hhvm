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

#ifndef incl_HPHP_VM_JIT_IRGEN_ITER_SPEC_H_
#define incl_HPHP_VM_JIT_IRGEN_ITER_SPEC_H_

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/extra-data.h"

namespace HPHP { namespace jit { namespace irgen {

//////////////////////////////////////////////////////////////////////

// Forward declaration: SpecializedIterator is used in IRGS, but we want to
// declare functions that take an IRGS& env as input.
struct IRGS;

// To reduce code size for specialized iters, we share blocks of generated code
// such as the "dec-ref old outputs; load, inc-ref, and store new ones" block
// that's part of both IterInit and IterNext.
//
// To make it possible to reuse this code, we consider IterInits / IterNexts
// equivalent if they share the same loop entry block, and we store a map from
// loop entry block -> SpecializedIterator struct in IRGS. See cpp for details.
struct SpecializedIterator {
  IterSpecialization iter_type;
  std::vector<IRInstruction*> placeholders;
  Block* header;
  Block* footer;
};

// If this method gens specialized code for an IterInit, it will hide it behind
// a placeholder, which we'll replace with a Jmp if we see a matching IterNext.
// Thus, we always need to emit generic code for IterInit as well.
//
// `doneOffset` is the relative offset to jump to if the base has no elements.
// `baseLocalId` is the base ID for local iters, or kInvalidId for non-local.
void specializeIterInit(IRGS& env, Offset doneOffset,
                        const IterArgs& data, uint32_t baseLocalId);

// Returns true on specialization. If it returns true, then we no longer need
// to emit generic code for this IterNext.
//
// `loopOffset` is the relative offset to jump to if the base has more elements.
// `baseLocalId` is the base ID for local iters, or kInvalidId for non-local.
bool specializeIterNext(IRGS& env, Offset loopOffset,
                        const IterArgs& data, uint32_t baseLocalId);

//////////////////////////////////////////////////////////////////////

}}}

#endif
