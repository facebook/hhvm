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

#include "hphp/runtime/base/rds-header.h"
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
 * Return a pointer to the type or value field of the pointee of `ptr', whether
 * it is a TPtrToCell or a TLvalToCell.
 */
Vptr memTVTypePtr(SSATmp* ptr, Vloc loc);
Vptr memTVValPtr(SSATmp* ptr, Vloc loc);

/*
 * Test or compare `s0' against the live type `s1', setting the result in `sf'.
 */
void emitTestTVType(Vout& v, Vreg sf, Immed s0, Vreg s1);
void emitTestTVType(Vout& v, Vreg sf, Immed s0, Vptr s1);
void emitCmpTVType(Vout& v, Vreg sf, DataType s0, Vptr s1);
void emitCmpTVType(Vout& v, Vreg sf, DataType s0, Vreg s1);

/*
 * Store `loc', the registers representing `src', to `dst'.
 */
void storeTV(Vout& v, Vptr dst, Vloc srcLoc, const SSATmp* src,
             Type ty = TBottom, bool aux = false);
void storeTV(Vout& v, Type type, Vloc srcLoc,
             Vptr typePtr, Vptr dataPtr, bool aux = false);

void storeTVWithAux(Vout& v, Vptr dst, Vloc srcLoc,
                    const SSATmp* src, AuxUnion aux);

void storeTVVal(Vout& v, Type type, Vloc srcLoc, Vptr valPtr);
void storeTVType(Vout& v, Type type, Vloc srcLoc,
                 Vptr typePtr, bool aux = false);

/*
 * Load `src' into `loc', the registers representing `dst'.
 *
 * If `aux' is true, we also need to load the m_aux field of the TypedValue
 * into the type reg.  This should only happen when loading a return value.
 */
void loadTV(Vout& v, const SSATmp* dst, Vloc dstLoc, Vptr src,
            bool aux = false);
void loadTV(Vout& v, Type type, Vloc dstLoc, Vptr typePtr, Vptr valPtr,
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
void trashFullTV(Vout& v, Vptr ptr, char byte);

/*
 * Fill the type and value of a TypedValue with trash.
 */
void trashTV(Vout& v, Vptr typePtr, Vptr valPtr, char byte);

/*
 * If the given type represents a statically known value, return
 * it. Return InvalidReg otherwise.
 */
Vreg materializeConstVal(Vout& v, Type ty);

/*
 * Compare an object's reference count with an immediate value, return the
 * status flags used for the comparison.
 */
Vreg emitCmpRefCount(Vout& v, Immed s0, Vreg s1);

/*
 * Store `s0' to the reference count of the given object.
 */
void emitStoreRefCount(Vout& v, Immed s0, Vreg s1);
void emitStoreRefCount(Vout& v, Immed s0, Vptr m);

/*
 * Decrement the reference count of the given object, returning the status
 * flags from the decrement instruction.
 */
Vreg emitDecRefCount(Vout& v, Vreg s0);

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
void emitIncRef(Vout& v, Vreg data, Reason reason);
Vreg emitDecRef(Vout& v, Vreg data, Reason reason);

/*
 * emitIncRefWork performs type check and calls incRef if appropriate.
 */
void emitIncRefWork(Vout& v, Vreg data, Vreg type, Reason reason);
void emitIncRefWork(Vout& v, Vloc loc, Type type, Reason reason);

/*
 * Check the refcount of `data'.  If it's negative (and hence, a sentinel
 * static value), do nothing.  If it's exactly 1, release `data' via the code
 * emitted by `destroy'.  Otherwise, decref it.
 *
 * We use `vcold' for the slow-path `destroy' if `unlikelyDestroy' is true.
 */
template<class Destroy>
void emitDecRefWork(Vout& v, Vout& vcold, Vreg data,
                    Destroy destroy, bool unlikelyDestroy,
                    Reason reason);

/*
 * Like emitDecRefWork(), but for a known-KindOfObject value.
 */
void emitDecRefWorkObj(Vout& v, Vreg obj, Reason reason);

/*
 * Trap or otherwise fail if `data' does not have a realistic refcount (either
 * a positive value or the sentinel static/uncounted values).
 */
void emitAssertRefCount(Vout& v, Vreg data, Reason reason);

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
Vptr lookupDestructor(Vout& v, Vreg type, bool typeIsQuad = false);

///////////////////////////////////////////////////////////////////////////////
// Class metadata.

/*
 * Load the Class* for `obj' into `d', then return `d'.
 */
Vreg emitLdObjClass(Vout& v, Vreg obj, Vreg d);

/*
 * Internal helpers for LowPtr comparisons.
 */
void cmpLowPtrImpl(Vout& v, Vreg sf, const void* ptr, Vptr mem, size_t size);
void cmpLowPtrImpl(Vout& v, Vreg sf, const void* ptr, Vreg reg, size_t size);
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
void emitCmpLowPtr(Vout& v, Vreg sf, const T* c, Vreg reg) {
  cmpLowPtrImpl(v, sf, c, reg, sizeof(LowPtr<T>));
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
 * emit an isCollection(obj) range check. The returned status
 * flags can be tested; CC_BE means true
 */
Vreg emitIsCollection(Vout& v, Vreg obj);

///////////////////////////////////////////////////////////////////////////////
// VM intrinsics.

/*
 * Set the VM register state.
 */
void emitSetVMRegState(Vout& v, VMRegState state);

/*
 * Write `msg' of type `t' to the global ring buffer.
 */
void emitRB(Vout& v, Trace::RingBufferType t, const char* msg);

/*
 * Increment the counter for `stat'.
 */
void emitIncStat(Vout& v, Stats::StatCounter stat);

///////////////////////////////////////////////////////////////////////////////
// RDS manipulation.

/*
 * Whether `ch' is initialized---i.e., whether its generation number matches
 * the current generation.
 *
 * @requires: rds::isNormalHandle(ch)
 */
Vreg checkRDSHandleInitialized(Vout& v, rds::Handle ch);
Vreg checkRDSHandleInitialized(Vout& v, Vreg ch);

/*
 * Update the generation number for `ch' to the current generation.
 *
 * @requires: rds::isNormalHandle(ch)
 */
void markRDSHandleInitialized(Vout& v, rds::Handle ch);
void markRDSHandleInitialized(Vout& v, Vreg ch);

void markRDSAccess(Vout& v, rds::Handle ch);
void markRDSAccess(Vout& v, Vreg ch);

///////////////////////////////////////////////////////////////////////////////
// Locals

/*
 * Obtain offsets of a local's type or value from the frame pointer.
 */
int offsetToLocalType(int id);
int offsetToLocalData(int id);

/*
 * Obtain a pointer to a local's type or value. Since the local's
 * index is statically known, this will never emit any code.
 */
Vptr ptrToLocalType(Vreg fp, int id);
Vptr ptrToLocalData(Vreg fp, int id);

/*
 * Given (valid) pointers to a local's type and value `typeIn' and
 * `dataIn', modify the pointers to point at the next local (by
 * increasing indx) and set `typeOUt' and `dataOut' to the new
 * pointers. It is up to the caller to detect when the pointers have
 * reached the end of the frame.
 */
void nextLocal(Vout& v,
               Vreg typeIn,
               Vreg dataIn,
               Vreg typeOut,
               Vreg dataOut,
               unsigned distance = 1);

/*
 * Given (valid) pointers to a local's type and value `typeIn' and
 * `dataIn', modify the pointers to point at the previous local (by
 * decreasing index) and set `typeOut' and `dataOut' to the new
 * pointers. It is up to the caller to detect when the pointers have
 * reached the end of the frame.
 */
void prevLocal(Vout& v,
               Vreg typeIn,
               Vreg dataIn,
               Vreg typeOut,
               Vreg dataOut);

///////////////////////////////////////////////////////////////////////////////

/*
 * Return a mask for an aux value suitable for ORing into the lower 64-bits of a
 * TypedValue.
 */
uint64_t auxToMask(AuxUnion);

}}

#include "hphp/runtime/vm/jit/code-gen-helpers-inl.h"
