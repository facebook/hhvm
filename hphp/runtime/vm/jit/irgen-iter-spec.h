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

#pragma once

#include "hphp/runtime/base/array-key-types.h"
#include "hphp/runtime/vm/jit/array-iter-profile.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/extra-data.h"

namespace HPHP::jit::irgen {

//////////////////////////////////////////////////////////////////////

// Forward declaration: SpecializedIterator is used in IRGS, but we want to
// declare functions that take an IRGS& env as input.
struct IRGS;

/*
 * Represents an information from profiling at IterInit, intersected with known
 * layout at that time. Used by IterNext to bootstrap its knowledge about the
 * iterated array, as known information about array specialization in frame
 * state is lost during irgen due to unprocessed preds while emitting the loop.
 */
struct IterProfileInfo {
  ArrayLayout layout;
};

// Generate specialized code for an IterInit. Returns true iff succeeded, which
// means we no longer need to emit the generic code.
//
// `doneOffset` is the relative offset to jump to if the base has no elements.
// `base` is the array base value
// `baseLocalId` is the local ID the `base` came from
bool specializeIterInit(IRGS& env, Offset doneOffset,
                        const IterArgs& data, SSATmp* base,
                        uint32_t baseLocalId,
                        ArrayIterProfile::Result profiledResult);

// Returns true on specialization. If it returns true, then we no longer need
// to emit generic code for this IterNext.
//
// `loopOffset` is the relative offset to jump to if the base has more elements.
// `base` is the array base value
// `baseLocalId` is the local ID the `base` came from
bool specializeIterNext(IRGS& env, Offset loopOffset,
                        const IterArgs& data, SSATmp* base,
                        uint32_t baseLocalId);

//////////////////////////////////////////////////////////////////////

}

