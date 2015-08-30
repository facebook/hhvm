/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VM_CODEGENHELPERS_X64_H_
#define incl_HPHP_VM_CODEGENHELPERS_X64_H_

#include "hphp/util/asm-x64.h"
#include "hphp/util/ringbuffer.h"

#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/align-x64.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/cpp-call.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

namespace HPHP {
//////////////////////////////////////////////////////////////////////

struct Func;

namespace jit {
//////////////////////////////////////////////////////////////////////

struct Fixup;
struct SSATmp;

namespace x64 {
//////////////////////////////////////////////////////////////////////

typedef X64Assembler Asm;

void emitEagerSyncPoint(Vout& v, const Op* pc, Vreg rds, Vreg vmfp, Vreg vmsp);
void emitGetGContext(Vout& as, Vreg dest);

void emitTransCounterInc(Asm& a);
void emitTransCounterInc(Vout&);

void emitAssertFlagsNonNegative(Vout& v, Vreg sf);

/*
 * Assuming rData is the data pointer for a refcounted (but possibly static)
 * value, emit a static check and DecRef, executing the code emitted by
 * `destroy' if the count would go to zero.
 */
template<class Destroy>
void emitDecRefWork(Vout& v, Vout& vcold, Vreg rData,
                    Destroy destroy, bool unlikelyDestroy) {
  auto const sf = v.makeReg();
  v << cmplim{1, rData[FAST_REFCOUNT_OFFSET], sf};
  ifThenElse(
    v, vcold, CC_E, sf,
    destroy,
    [&] (Vout& v) {
      // If it's not static, actually reduce the reference count.  This does
      // another branch using the same status flags from the cmplim above.
      ifThen(
        v, CC_NL, sf,
        [&] (Vout& v) {
          auto const sf = v.makeReg();
          v << declm{rData[FAST_REFCOUNT_OFFSET], sf};
          emitAssertFlagsNonNegative(v, sf);
        }
      );
    },
    unlikelyDestroy
  );
}

void emitIncRef(Vout& v, Vreg base);

void emitAssertFlagsNonNegative(Vout& v, Vreg sf);
void emitAssertRefCount(Vout& v, Vreg base);

Vreg emitLdObjClass(Vout& v, Vreg objReg, Vreg dstReg);
Vreg emitLdClsCctx(Vout& v, Vreg srcReg, Vreg dstReg);

void emitCall(Asm& as, TCA dest, RegSet args);
void emitCall(Asm& as, CppCall call, RegSet args);
void emitCall(Vout& v, CppCall call, RegSet args);

// store imm to the 8-byte memory location at ref. Warning: don't use this
// if you wanted an atomic store; large imms cause two stores.
void emitImmStoreq(Vout& v, Immed64 imm, Vptr ref);

void emitRB(Vout& v, Trace::RingBufferType t, const char* msg);

void emitCheckSurpriseFlagsEnter(Vout& main, Vout& cold, Vreg fp, Vreg rds,
                                 Fixup fixup, Vlabel catchBlock);

/*
 * Direct access to __thread variables.
 * we support x64 linux (linked off FS), and x64 MacOS using
 * some reverse engineering of their generated code.
 */

#ifndef __APPLE__
/*
 * x86 terminology review: "Virtual addresses" are subject to both
 * segmented translation and paged translation. "Linear addresses" are
 * post-segmentation address, subject only to paging. C and C++ generally
 * only have access to bitwise linear addresses.
 *
 * On Linux, TLS data live at negative virtual addresses off FS: the first datum
 * is typically at VA(FS:-sizeof(datum)). Linux's x64 ABI stores the linear
 * address of the base of TLS at VA(FS:0). While this is just a convention, it
 * is firm: gcc builds binaries that assume it when, e.g., evaluating
 * "&myTlsDatum".
 *
 * The virtual addresses of TLS data are not exposed to C/C++. To figure it
 * out, we take a datum's linear address, and subtract it from the linear
 * address where TLS starts.
 */
namespace detail {
template<typename T>
inline Vptr getTLSVptr(const T& data) {
  uintptr_t virtualAddress = uintptr_t(&data) - tlsBase();
  return Vptr{baseless(virtualAddress), Vptr::FS};
}

template<typename T>
inline Vptr
implTLSAddr(Vout& v, T& data, Vreg) {
  return detail::getTLSVptr(data);
}

template<typename T>
inline Vptr
implTLSAddr(X64Assembler& a, T& data, Reg64) {
   return detail::getTLSVptr(data);
}
}
#else
/*
 * In MacOS an __thread variable has a "key" allocated in global memory, under
 * the name of the object.  The key consists of three longs. The first holds
 * the address of a function, which if called with $rdi pointing at the key (ie
 * $rdi=&key; call 0($rdi)) will return the address of the thread local
 * (possibly allocating it, if this is the first time its been accessed in the
 * current thread).
 *
 * The function is very short (in the case that the thread local has already
 * been allocated), and preserves all the registers except for $rax (which gets
 * the address of the thread local) - so we could just use it to access the
 * thread locals. But we can do better.
 *
 * The second long contains a gs-relative index to a pointer to the block of
 * memory containing the thread-local, and the third long contains the offset
 * to it within that block. So if $rdi points at the key,
 *
 *  movq 8($rdi), $rax
 *  movq gs:(,$rax,8), $rax
 *  addq 16($rdi), $rax
 *
 * will get us the address of the thread local.
 *
 * But we can still do better. key[1] and key[2] are constants for the duration
 * of the program; so we can burn their values into the tc:
 *
 *  movq gs:{key[1]*8}, $rax
 *  addq {key[2]}, $rax
 *
 * But note that the add will often fold into a following load or store.
 *
 * So the only remaining problem is to get the address of the key:
 *
 *   __asm__("lea %1, %0" : "=r"(ret) : "m"(tlvar));
 *
 * unfortunately, clang is too smart here, and converts the lea into:
 *
 *  lea _tlvar, $rdi
 *  call ($rdi)
 *  move $rax, ret
 *
 * Fortunately, the helper preserves all regs (except $rax), and so $rdi now
 * has the address of the key. We use that trick in getGlobalAddrForTls.
 *
 * Finally note that all of this is only valid if the thread local has already
 * been accessed in the current thread - but we can easily ensure that (its
 * already true for any ThreadLocalNoCheck variable).
 */
namespace detail {

inline Vptr
implTLSAddr(Vout& v, long* addr, Vreg scratch) {
  v << load{Vptr{baseless(addr[1] * 8), Vptr::GS}, scratch};
  return scratch[addr[2]];
}

inline Vptr
implTLSAddr(Asm& a, long* addr, Reg64 scratch) {
  a.gs().loadq(baseless(addr[1] * 8), scratch);
  return scratch[addr[2]];
}

}

#endif

#ifdef USE_GCC_FAST_TLS

/* Access to ThreadLocalNoCheck variables with USE_GCC_FAST_TLS
 *
 * We support both linux and MacOS
 */

namespace detail {
#ifndef __APPLE__

template<typename T>
inline void
implTLSLoad(Vout& v, const ThreadLocalNoCheck<T>& datum,
            long* unused, Vreg reg) {
  v << load{detail::getTLSVptr(datum.m_node.m_p), reg};
}

template<typename T>
inline void
implTLSLoad(X64Assembler& a, const ThreadLocalNoCheck<T>& datum,
            long* unused, Reg64 reg) {
  auto ptr = detail::getTLSVptr(datum.m_node.m_p);
  Vasm::prefix(a, ptr).loadq(ptr.mr(), reg);
}

#else

template<typename T>
inline void
implTLSLoad(Vout& v, const ThreadLocalNoCheck<T>& datum, long* addr, Vreg dst) {
  auto tmp = v.makeReg();
  auto ptr = detail::implTLSAddr(v, addr, tmp);
  v << load{ptr, dst};
}

template<typename T>
inline void
implTLSLoad(X64Assembler& a, const ThreadLocalNoCheck<T>& datum,
            long* addr, Reg64 dst) {
  auto ptr = detail::implTLSAddr(a, addr, dst);
  Vasm::prefix(a, ptr).loadq(ptr, dst);
}

#endif
}

#else // USE_GCC_FAST_TLS

template<typename T>
inline void
emitTLSLoad(Vout& v, const ThreadLocalNoCheck<T>& datum, Vreg dest) {
  // We don't know for sure what's alive.
  PhysRegSaver(v, abi().gpUnreserved - abi().calleeSaved);
  v << ldimmq{datum.m_key, rarg(0)};
  const CodeAddress addr = (CodeAddress)pthread_getspecific;
  v << call{addr, arg_regs(1)};
  if (dest != Vreg(reg::rax)) {
    v << copy{reg::rax, dest};
  }
}

#endif // USE_GCC_FAST_TLS

// Emit a load of a low pointer.
void emitLdLowPtr(Vout& v, Vptr mem, Vreg reg, size_t size);

void emitCmpClass(Vout& v, Vreg sf, const Class* c, Vptr mem);
void emitCmpClass(Vout& v, Vreg sf, Vreg reg, Vptr mem);
void emitCmpClass(Vout& v, Vreg sf, Vreg reg1, Vreg reg2);

void emitCmpVecLen(Vout& v, Vreg sf, Vptr mem, Immed val);

void copyTV(Vout& v, Vloc src, Vloc dst, Type destType);
void pack2(Vout& v, Vreg s0, Vreg s1, Vreg d0);

Vreg zeroExtendIfBool(Vout& v, const SSATmp* src, Vreg reg);

template<ConditionCode Jcc, class Lambda>
void jccBlock(Asm& a, Lambda body) {
  Label exit;
  exit.jcc8(a, Jcc);
  body();
  asm_label(a, exit);
}

/*
 * lookupDestructor --
 *
 * Return a MemoryRef pointer to the destructor for the type in typeReg.
 */

inline MemoryRef lookupDestructor(X64Assembler& a, PhysReg typeReg) {
  auto const table = reinterpret_cast<intptr_t>(g_destructors);
  always_assert_flog(deltaFits(table, sz::dword),
    "Destructor function table is expected to be in the data "
    "segment, with addresses less than 2^31"
  );
  static_assert((KindOfString        >> kShiftDataTypeToDestrIndex == 1) &&
                (KindOfArray         >> kShiftDataTypeToDestrIndex == 2) &&
                (KindOfObject        >> kShiftDataTypeToDestrIndex == 3) &&
                (KindOfResource      >> kShiftDataTypeToDestrIndex == 4) &&
                (KindOfRef           >> kShiftDataTypeToDestrIndex == 5),
                "lookup of destructors depends on KindOf* values");
  a.    shrl   (kShiftDataTypeToDestrIndex, r32(typeReg));
  return baseless(typeReg*8 + table);
}

inline Vptr lookupDestructor(Vout& v, Vreg typeReg) {
  auto const table = reinterpret_cast<intptr_t>(g_destructors);
  always_assert_flog(deltaFits(table, sz::dword),
    "Destructor function table is expected to be in the data "
    "segment, with addresses less than 2^31"
  );
  static_assert((KindOfString        >> kShiftDataTypeToDestrIndex == 1) &&
                (KindOfArray         >> kShiftDataTypeToDestrIndex == 2) &&
                (KindOfObject        >> kShiftDataTypeToDestrIndex == 3) &&
                (KindOfResource      >> kShiftDataTypeToDestrIndex == 4) &&
                (KindOfRef           >> kShiftDataTypeToDestrIndex == 5),
                "lookup of destructors depends on KindOf* values");
  auto shiftedType = v.makeReg();
  v << shrli{kShiftDataTypeToDestrIndex, typeReg, shiftedType, v.makeReg()};
  return Vptr{Vreg{}, shiftedType, 8, safe_cast<int>(table)};
}

inline ptrdiff_t genOffset(bool isAsync) {
  return isAsync ? AsyncGenerator::objectOff() : Generator::objectOff();
}

//////////////////////////////////////////////////////////////////////

}}}

#endif
