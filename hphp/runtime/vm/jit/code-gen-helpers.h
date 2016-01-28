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

#ifndef incl_HPHP_VM_CODE_GEN_HELPERS_H_
#define incl_HPHP_VM_CODE_GEN_HELPERS_H_

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
 * Copy two 64-bit values, `s0' and `s1', into one 128-bit register, `d0'.
 */
void pack2(Vout& v, Vreg s0, Vreg s1, Vreg d0);

/*
 * Zero-extend `reg' if `src' might be a bool, returning the dest Vreg.
 */
Vreg zeroExtendIfBool(Vout& v, const SSATmp* src, Vreg reg);

///////////////////////////////////////////////////////////////////////////////
// TypedValue manipulations.

/*
 * Load the TV type at `mem' into `d'.
 */
void emitLoadTVType(Vout& v, Vptr mem, Vreg8 d);

/*
 * Test or compare `s0' against the live type `s1', setting the result in `sf'.
 */
void emitTestTVType(Vout& v, Vreg sf, Immed s0, Vreg s1);
void emitTestTVType(Vout& v, Vreg sf, Immed s0, Vptr s1);
void emitCmpTVType(Vout& v, Vreg sf, Immed s0, Vptr s1);
void emitCmpTVType(Vout& v, Vreg sf, Immed s0, Vreg s1);

/*
 * Copy the TV in `src' to `dst'.
 */
void copyTV(Vout& v, Vloc src, Vloc dst, Type destType);

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
 * DecRef for KindOfObject type, and release the object if necessary.  This
 * DecRef optimizes for known KindOfObject (the case in AsyncRetCtrl stub).
 */
void emitDecRefObj(Vout& v, Vreg obj);

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
 * Trap or otherwise fail if `data' does not have a realistic refcount (either
 * a positive value or the sentinel static/uncounted values).
 */
void emitAssertRefCount(Vout& v, Vreg data);

///////////////////////////////////////////////////////////////////////////////
// Calls.

void emitCall(Vout& v, CallSpec call, RegSet args);

/*
 * Return a Vptr to the native destructor function for values of type `type'.
 */
Vptr lookupDestructor(Vout& v, Vreg type);

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
 * Compare two classes, setting the result in `sf'.
 */
void emitCmpClass(Vout& v, Vreg sf, const Class* c, Vptr mem);
void emitCmpClass(Vout& v, Vreg sf, Vreg reg, Vptr mem);
void emitCmpClass(Vout& v, Vreg sf, Vreg reg1, Vreg reg2);

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
 * Atomically increment the translation counter for the translation we are
 * currently emitting.
 */
void emitTransCounterInc(Vout& v);

/*
 * Write `msg' of type `t' to the global ring buffer.
 */
void emitRB(Vout& v, Trace::RingBufferType t, const char* msg);

///////////////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/vm/jit/code-gen-helpers-inl.h"

#endif
