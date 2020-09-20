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

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/trans-rec.h"

namespace HPHP { namespace jit { namespace FuncOrder {

/*
 * Compute the ordering for optimized functions using hfsort on the call graph.
 * Also returns the average profile count across all profiled blocks.
 */
uint64_t compute();

/*
 * Get the cached function order.
 */
const std::vector<FuncId>& get();

/*
 * Serialize/deserialize the cached function order.
 */
void serialize(ProfDataSerializer&);
void deserialize(ProfDataDeserializer&);

/*
 * Set/get/clear the function associated with the given call-return address.
 */
void   setCallFuncId(TCA callRetAddr, FuncId funcId);
FuncId getCallFuncId(TCA callRetAddr);
void   clearCallFuncId(TCA callRetAddr);

/*
 * Increment the count for the top-most call in the stack at `fp'.
 */
void incCount(const ActRec* fp);

/*
 * Record that a translation is emitted so that the size of functions can be
 * calculated.
 */
void recordTranslation(const TransRec& transRec);

} } }

