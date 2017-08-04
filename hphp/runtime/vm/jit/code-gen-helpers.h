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

#ifndef incl_HPHP_VM_CODE_GEN_HELPERS_H_
#define incl_HPHP_VM_CODE_GEN_HELPERS_H_

#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/hhbc.h"

#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/immed.h"
#include "hphp/util/ringbuffer.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct Class;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct SSATmp;
struct Vout;

///////////////////////////////////////////////////////////////////////////////
// General purpose.

/*
 * Store `imm' to the 8-byte memory location at `ref'.
 *
 * We use a helper for this operation because the `storeqi' Vinstr can only
 * take a 32-bit Immed, but we want to use it if possible instead of `store'.
 */
void emitImmStoreq(Vout& v, Immed64 imm, Vptr ref);

/*
 * Load the LowPtr<T> at `mem', with storage size `size', into `reg'.
 */
void emitLdLowPtr(Vout& v, Vptr mem, Vreg reg, size_t size);

/*
 * Store the LowPtr<T> in `reg' into `mem', with storage size `size'.
 */
void emitStLowPtr(Vout& v, Vreg reg, Vptr mem, size_t size);

/*
 * Copy two 64-bit values, `s0' and `s1', into one 128-bit register, `d0'.
 */
void pack2(Vout& v, Vreg s0, Vreg s1, Vreg d0);

/*
 * Zero-extend `reg' if `type' is a bool, returning the dest Vreg.
 */
Vreg zeroExtendIfBool(Vout& v, Type type, Vreg reg);

///////////////////////////////////////////////////////////////////////////////
// TypedValue manipulations.

/*
 * Test or compare `s0' against the live type `s1', setting the result in `sf'.
 */
void emitTestTVType(Vout& v, Vreg sf, Immed s0, Vreg s1);
void emitTestTVType(Vout& v, Vreg sf, Immed s0, Vptr s1);
void emitCmpTVType(Vout& v, Vreg sf, Immed s0, Vptr s1);
void emitCmpTVType(Vout& v, Vreg sf, Immed s0, Vreg s1);

Vreg emitMaskTVType(Vout& v, Immed s0, Vreg s1);
Vreg emitMaskTVType(Vout& v, Immed s0, Vptr s1);

/*
 * Store `loc', the registers representing `src', to `dst'.
 */
void storeTV(Vout& v, Vptr dst, Vloc srcLoc, const SSATmp* src);

/*
 * Load `src' into `loc', the registers representing `dst'.
 *
 * If `aux' is true, we also need to load the m_aux field of the TypedValue
 * into the type reg.  This should only happen when loading a return value.
 */
void loadTV(Vout& v, const SSATmp* dst, Vloc dstLoc, Vptr src,
            bool aux = false);

/*
 * Copy the TV in `src' into `dst', or into `data' and `type'.
 */
void copyTV(Vout& v, Vreg data, Vreg type, Vloc srcLoc, const SSATmp* src);
void copyTV(Vout& v, Vloc src, Vloc dst, Type dstType);

/*
 * Fill all the bytes of a TypedValue with trash.
 *
 * Note that this will also clobber the Aux area of a TypedValueAux.
 */
void trashTV(Vout& v, Vreg ptr, int32_t offset, char byte);

/*
 * Incref or decref `data', and perform some asserts.
 *
 * These routines do not perform any checks; they just unconditionally mutate
 * the value's refcount.
 *
 * emitDecRef() returns the status flags register that results from the
 * decrement, since callers may want to check the result and invoke data
 * destructors.
 */
void emitIncRef(Vout& v, Vreg data);
Vreg emitDecRef(Vout& v, Vreg data);

/*
 * emitIncRefWork performs type check and calls incRef if appropriate.
 */
void emitIncRefWork(Vout& v, Vreg data, Vreg type);

/*
 * Check the refcount of `data'.  If it's negative (and hence, a sentinel
 * static value), do nothing.  If it's exactly 1, release `data' via the code
 * emitted by `destroy'.  Otherwise, decref it.
 *
 * We use `vcold' for the slow-path `destroy' if `unlikelyDestroy' is true.
 */
template<class Destroy>
void emitDecRefWork(Vout& v, Vout& vcold, Vreg data,
                    Destroy destroy, bool unlikelyDestroy);

/*
 * Like emitDecRefWork(), but for a known-KindOfObject value.
 */
void emitDecRefWorkObj(Vout& v, Vreg obj);

/*
 * Trap or otherwise fail if `data' does not have a realistic refcount (either
 * a positive value or the sentinel static/uncounted values).
 */
void emitAssertRefCount(Vout& v, Vreg data);

///////////////////////////////////////////////////////////////////////////////
// Calls.

/*
 * Emit a non-PHP function call.
 *
 * @see: call{} and similar
 */
void emitCall(Vout& v, CallSpec call, RegSet args);

/*
 * Return a Vptr to the native destructor function for values of type `type'.
 */
Vptr lookupDestructor(Vout& v, Vreg type, bool typeIsLong = false);

///////////////////////////////////////////////////////////////////////////////
// Class metadata.

/*
 * Load the Class* for `obj' into `d', then return `d'.
 */
Vreg emitLdObjClass(Vout& v, Vreg obj, Vreg d);

/*
 * Load the Class* underlying the Cctx `src' into `d', then return `d'.
 *
 * (This just unmasks the lowest-order bit, which designates `src' as a Cctx
 * rather than a This.)
 */
Vreg emitLdClsCctx(Vout& v, Vreg src, Vreg d);

/*
 * Internal helpers for LowPtr comparisons.
 */
void cmpLowPtrImpl(Vout& v, Vreg sf, const void* ptr, Vptr mem, size_t size);
void cmpLowPtrImpl(Vout& v, Vreg sf, Vreg reg, Vptr mem, size_t size);
void cmpLowPtrImpl(Vout& v, Vreg sf, Vreg reg1, Vreg reg2, size_t size);

/*
 * Compare two LowPtrs, setting the result in `sf'.
 */
template<class T>
void emitCmpLowPtr(Vout& v, Vreg sf, const T* c, Vptr mem) {
  cmpLowPtrImpl(v, sf, c, mem, sizeof(LowPtr<T>));
}

template<class T>
void emitCmpLowPtr(Vout& v, Vreg sf, Vreg reg, Vptr mem) {
  cmpLowPtrImpl(v, sf, reg, mem, sizeof(LowPtr<T>));
}

template<class T>
void emitCmpLowPtr(Vout& v, Vreg sf, Vreg reg1, Vreg reg2) {
  cmpLowPtrImpl(v, sf, reg1, reg2, sizeof(LowPtr<T>));
}

/*
 * Compare `val' against the live Class::veclen_t at `mem'.
 */
void emitCmpVecLen(Vout& v, Vreg sf, Immed val, Vptr mem);

///////////////////////////////////////////////////////////////////////////////
// VM intrinsics.

/*
 * Eagerly sync the vm regs to RDS.
 */
void emitEagerSyncPoint(Vout& v, PC pc, Vreg rds, Vreg vmfp, Vreg vmsp);

/*
 * Write `msg' of type `t' to the global ring buffer.
 */
void emitRB(Vout& v, Trace::RingBufferType t, const char* msg);

/*
 * Increment the counter for `stat' by `n'.
 *
 * If `force' is set, do so even if stats aren't enabled.
 */
void emitIncStat(Vout& v, Stats::StatCounter stat, int n = 1,
                 bool force = false);

///////////////////////////////////////////////////////////////////////////////
// RDS manipulation.

/*
 * Whether `ch' is initialized---i.e., whether its generation number matches
 * the current generation.
 *
 * @requires: rds::isNormalHandle(ch)
 */
Vreg checkRDSHandleInitialized(Vout& v, rds::Handle ch);

/*
 * Update the generation number for `ch' to the current generation.
 *
 * @requires: rds::isNormalHandle(ch)
 */
void markRDSHandleInitialized(Vout& v, rds::Handle ch);

}}

#include "hphp/runtime/vm/jit/code-gen-helpers-inl.h"

#endif
