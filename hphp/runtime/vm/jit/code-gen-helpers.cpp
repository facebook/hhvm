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

#include "hphp/runtime/vm/jit/code-gen-helpers.h"

#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/vm/class.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/abi-cxx.h"
#include "hphp/util/immed.h"
#include "hphp/util/low-ptr.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/thread-local.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhir);

namespace {

///////////////////////////////////////////////////////////////////////////////

void assertSFNonNegative(Vout& v, Vreg sf) {
  if (!RuntimeOption::EvalHHIRGenerateAsserts) return;
  ifThen(v, CC_NGE, sf, [&] (Vout& v) { v << ud2{}; });
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

Vreg emitMovtql(Vout& v, Vreg reg) {
  auto it = v.unit().regToConst.find(reg);
  if (it != v.unit().regToConst.end() && !it->second.isUndef) {
    switch (it->second.kind) {
      case Vconst::Double:
        always_assert(false);
      case Vconst::Quad:
        return v.unit().makeConst(uint32_t(it->second.val));
      case Vconst::Long:
      case Vconst::Byte:
        return reg;
    }
  }
  auto const r = v.makeReg();
  v << movtql{reg, r};
  return r;
}

void emitImmStoreq(Vout& v, Immed64 imm, Vptr ref) {
  if (imm.fits(sz::dword)) {
    v << storeqi{imm.l(), ref};
  } else {
    v << store{v.cns(imm.q()), ref};
  }
}

void emitLdLowPtr(Vout& v, Vptr mem, Vreg reg, size_t size) {
  if (size == 8) {
    v << load{mem, reg};
  } else if (size == 4) {
    v << loadzlq{mem, reg};
  } else {
    not_implemented();
  }
}

void emitStLowPtr(Vout& v, Vreg reg, Vptr mem, size_t size) {
  if (size == 8) {
    v << store{reg, mem};
  } else if (size == 4) {
    auto const temp = emitMovtql(v, reg);
    v << storel{temp, mem};
  } else {
    not_implemented();
  }
}

void pack2(Vout& v, Vreg s0, Vreg s1, Vreg d0) {
  auto prep = [&] (Vreg r) {
    if (VregDbl::allowable(r)) return r;
    auto t = v.makeReg();
    v << copy{r, t};
    return t;
  };
  // s0 and s1 must be valid VregDbl registers; prep() takes care of it.
  v << unpcklpd{prep(s1), prep(s0), d0}; // s0,s1 -> d0[0],d0[1]
}

Vreg zeroExtendIfBool(Vout& v, Type ty, Vreg reg) {
  if (!(ty <= TBool)) return reg;

  // Zero-extend the bool from a byte to a quad.
  auto extended = v.makeReg();
  v << movzbq{reg, extended};
  return extended;
}

///////////////////////////////////////////////////////////////////////////////

void storeTV(Vout& v, Vptr dst, Vloc srcLoc, const SSATmp* src) {
  auto const type = src->type();

  if (srcLoc.isFullSIMD()) {
    // The whole TV is stored in a single SIMD reg.
    assertx(RuntimeOption::EvalHHIRAllocSIMDRegs);
    v << storeups{srcLoc.reg(), dst};
    return;
  }

  if (type.needsReg()) {
    assertx(srcLoc.hasReg(1));
    v << storeb{srcLoc.reg(1), dst + TVOFF(m_type)};
  } else {
    v << storeb{v.cns(type.toDataType()), dst + TVOFF(m_type)};
  }

  // We ignore the values of statically nullish types.
  if (src->isA(TNull) || src->isA(TNullptr)) return;

  // Store the value.
  if (src->hasConstVal()) {
    // Skip potential zero-extend if we know the value.
    v << store{v.cns(src->rawVal()), dst + TVOFF(m_data)};
  } else {
    assertx(srcLoc.hasReg(0));
    auto const extended = zeroExtendIfBool(v, src->type(), srcLoc.reg(0));
    v << store{extended, dst + TVOFF(m_data)};
  }
}

void loadTV(Vout& v, const SSATmp* dst, Vloc dstLoc, Vptr src,
            bool aux /* = false */) {
  auto const type = dst->type();

  if (dstLoc.isFullSIMD()) {
    // The whole TV is loaded into a single SIMD reg.
    assertx(RuntimeOption::EvalHHIRAllocSIMDRegs);
    v << loadups{src, dstLoc.reg()};
    return;
  }

  if (type.needsReg()) {
    assertx(dstLoc.hasReg(1));
    if (aux) {
      v << load{src + TVOFF(m_type), dstLoc.reg(1)};
    } else {
      v << loadb{src + TVOFF(m_type), dstLoc.reg(1)};
    }
  }

  if (type <= TBool) {
    v << loadtqb{src + TVOFF(m_data), dstLoc.reg(0)};
  } else {
    v << load{src + TVOFF(m_data), dstLoc.reg(0)};
  }
}

void copyTV(Vout& v, Vreg data, Vreg type, Vloc srcLoc, const SSATmp* src) {
  // SIMD register are not supported here.
  assertx(!srcLoc.isFullSIMD());

  if (src->type().needsReg()) {
    assertx(srcLoc.hasReg(1));
    v << copy{srcLoc.reg(1), type};
  } else {
    v << copy{v.cns(src->type().toDataType()), type};
  }

  // Ignore the values for nulls.
  if (src->isA(TNull)) return;

  if (src->hasConstVal()) {
    // Skip potential zero-extend if we know the value.
    v << copy{v.cns(src->rawVal()), data};
  } else {
    assertx(srcLoc.hasReg(0));
    auto const extended = zeroExtendIfBool(v, src->type(), srcLoc.reg(0));
    v << copy{extended, data};
  }
}

void copyTV(Vout& v, Vloc src, Vloc dst, Type destType) {
  auto src_arity = src.numAllocated();
  auto dst_arity = dst.numAllocated();

  if (dst_arity == 2) {
    always_assert(src_arity == 2);
    v << copy2{src.reg(0), src.reg(1), dst.reg(0), dst.reg(1)};
    return;
  }
  always_assert(dst_arity == 1);

  if (src_arity == 2 && dst.isFullSIMD()) {
    pack2(v, src.reg(0), src.reg(1), dst.reg(0));
    return;
  }
  always_assert(src_arity >= 1);

  if (src_arity == 2 && destType <= TBool) {
    v << movtqb{src.reg(0), dst.reg(0)};
  } else {
    v << copy{src.reg(0), dst.reg(0)};
  }
}

void trashTV(Vout& v, Vreg ptr, int32_t offset, char byte) {
  int32_t trash32;
  memset(&trash32, byte, sizeof(trash32));
  static_assert(sizeof(TypedValue) == 16, "");
  v << storeli{trash32, ptr[offset + 0x0]};
  v << storeli{trash32, ptr[offset + 0x4]};
  v << storeli{trash32, ptr[offset + 0x8]};
  v << storeli{trash32, ptr[offset + 0xc]};
}

void emitAssertRefCount(Vout& v, Vreg data) {
  auto const sf = v.makeReg();
  v << cmplim{StaticValue, data[FAST_REFCOUNT_OFFSET], sf};

  ifThen(v, CC_NLE, sf, [&] (Vout& v) {
    auto const sf = v.makeReg();
    v << cmplim{RefCountMaxRealistic, data[FAST_REFCOUNT_OFFSET], sf};

    ifThen(v, CC_NBE, sf, [&] (Vout& v) { v << ud2{}; });
  });
}

void emitIncRef(Vout& v, Vreg base) {
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitAssertRefCount(v, base);
  }
  auto const sf = v.makeReg();
  v << inclm{base[FAST_REFCOUNT_OFFSET], sf};
  assertSFNonNegative(v, sf);
}

Vreg emitDecRef(Vout& v, Vreg base) {
  auto const sf = v.makeReg();
  v << declm{base[FAST_REFCOUNT_OFFSET], sf};
  assertSFNonNegative(v, sf);

  return sf;
}

void emitIncRefWork(Vout& v, Vreg data, Vreg type) {
  auto const sf = v.makeReg();
  emitCmpTVType(v, sf, KindOfRefCountThreshold, type);
  // ifRefCountType
  ifThen(v, CC_G, sf, [&] (Vout& v) {
    auto const sf2 = v.makeReg();
    // ifNonStatic
    v << cmplim{0, data[FAST_REFCOUNT_OFFSET], sf2};
    ifThen(v, CC_GE, sf2, [&] (Vout& v) { emitIncRef(v, data); });
  });
}

void emitDecRefWorkObj(Vout& v, Vreg obj) {
  auto const shouldRelease = v.makeReg();
  v << cmplim{1, obj[FAST_REFCOUNT_OFFSET], shouldRelease};
  ifThenElse(
    v, CC_E, shouldRelease,
    [&] (Vout& v) {
      // Put fn inside vcall{} triggers a compiler internal error (gcc 4.4.7)
      auto const fn = CallSpec::method(&ObjectData::release);
      v << vcall{fn, v.makeVcallArgs({{obj}}), v.makeTuple({})};
    },
    [&] (Vout& v) {
      emitDecRef(v, obj);
    }
  );
}

///////////////////////////////////////////////////////////////////////////////

void emitCall(Vout& v, CallSpec target, RegSet args) {
  using K = CallSpec::Kind;

  switch (target.kind()) {
    case K::Direct:
      v << call{static_cast<TCA>(target.address()), args};
      return;

    case K::Smashable:
      v << calls{static_cast<TCA>(target.address()), args};
      return;

    case K::ArrayVirt: {
      auto const addr = reinterpret_cast<intptr_t>(target.arrayTable());

      auto const arrkind = v.makeReg();
      v << loadzbl{rarg(0)[HeaderKindOffset], arrkind};

      if (deltaFits(addr, sz::dword)) {
        v << callm{baseless(arrkind * 8 + addr), args};
      } else {
        auto const base = v.makeReg();
        v << ldimmq{addr, base};
        v << callm{base[arrkind * 8], args};
      }
      static_assert(sizeof(HeaderKind) == 1, "");
    } return;

    case K::Destructor: {
      auto dtor = lookupDestructor(v, target.reg());
      v << callm{dtor, args};
    } return;

    case K::Stub:
      v << callstub{target.stubAddr(), args};
      return;
  }
  not_reached();
}

Vptr lookupDestructor(Vout& v, Vreg type, bool typeIsLong) {
  auto const table = reinterpret_cast<intptr_t>(g_destructors);

  auto const index = v.makeReg();
  auto const indexl = v.makeReg();
  auto const typel = [&] {
    if (!typeIsLong) {
      auto r = v.makeReg();
      // the caller didn't zero extend the type, so we need to here
      v << movzbl{type, r};
      return r;
    }
    return type;
  }();

  v << shrli{kShiftDataTypeToDestrIndex, typel, indexl, v.makeReg()};
  v << movzlq{indexl, index};

  // The baseless form is more compact, but isn't supported for 64-bit
  // displacements.
  if (table <= std::numeric_limits<int>::max()) {
    return baseless(index * 8 + safe_cast<int>(table));
  }
  return v.cns(table)[index * 8];
}

///////////////////////////////////////////////////////////////////////////////

Vreg emitLdObjClass(Vout& v, Vreg obj, Vreg d) {
  emitLdLowPtr(v, obj[ObjectData::getVMClassOffset()], d,
               sizeof(LowPtr<Class>));
  return d;
}

Vreg emitLdClsCctx(Vout& v, Vreg src, Vreg dst) {
  static_assert(ActRec::kHasClassBit == 1,
                "Fix the decq if you change kHasClassBit");
  v << decq{src, dst, v.makeReg()};
  return dst;
}

void cmpLowPtrImpl(Vout& v, Vreg sf, const void* ptr, Vptr mem, size_t size) {
  if (size == 8) {
    v << cmpqm{v.cns(ptr), mem, sf};
  } else if (size == 4) {
    auto const ptrImm = safe_cast<uint32_t>(reinterpret_cast<intptr_t>(ptr));
    v << cmplm{v.cns(ptrImm), mem, sf};
  } else {
    not_implemented();
  }
}

void cmpLowPtrImpl(Vout& v, Vreg sf, Vreg reg, Vptr mem, size_t size) {
  if (size == 8) {
    v << cmpqm{reg, mem, sf};
  } else if (size == 4) {
    auto low = emitMovtql(v, reg);
    v << cmplm{low, mem, sf};
  } else {
    not_implemented();
  }
}

void cmpLowPtrImpl(Vout& v, Vreg sf, Vreg reg1, Vreg reg2, size_t size) {
  if (size == 8) {
    v << cmpq{reg1, reg2, sf};
  } else if (size == 4) {
    auto const l1 = emitMovtql(v, reg1);
    auto const l2 = emitMovtql(v, reg2);
    v << cmpl{l1, l2, sf};
  } else {
    not_implemented();
  }
}

void emitCmpVecLen(Vout& v, Vreg sf, Immed val, Vptr mem) {
  auto const size = sizeof(Class::veclen_t);
  if (size == 2) {
    v << cmpwim{val, mem, sf};
  } else if (size == 4) {
    v << cmplim{val, mem, sf};
  } else {
    not_implemented();
  }
}

///////////////////////////////////////////////////////////////////////////////

void emitEagerSyncPoint(Vout& v, PC pc, Vreg rds, Vreg vmfp, Vreg vmsp) {
  v << store{vmfp, rds[rds::kVmfpOff]};
  v << store{vmsp, rds[rds::kVmspOff]};
  emitImmStoreq(v, intptr_t(pc), rds[rds::kVmpcOff]);
}

void emitRB(Vout& v, Trace::RingBufferType t, const char* msg) {
  if (!Trace::moduleEnabled(Trace::ringbuffer, 1)) return;
  v << vcall{CallSpec::direct(Trace::ringbufferMsg),
             v.makeVcallArgs({{v.cns(msg), v.cns(strlen(msg)), v.cns(t)}}),
             v.makeTuple({})};
}

void emitIncStat(Vout& v, Stats::StatCounter stat, int n, bool force) {
  if (!force && !Stats::enabled()) return;
  intptr_t disp = uintptr_t(&Stats::tl_counters[stat]) - tlsBase();
  v << addqim{n, Vptr{baseless(disp), Vptr::FS}, v.makeReg()};
}

///////////////////////////////////////////////////////////////////////////////

Vreg checkRDSHandleInitialized(Vout& v, rds::Handle ch) {
  assertx(rds::isNormalHandle(ch));
  auto const gen = v.makeReg();
  auto const sf = v.makeReg();
  v << loadb{rvmtl()[rds::genNumberHandleFrom(ch)], gen};
  v << cmpbm{gen, rvmtl()[rds::currentGenNumberHandle()], sf};
  return sf;
}

void markRDSHandleInitialized(Vout& v, rds::Handle ch) {
  assertx(rds::isNormalHandle(ch));
  auto const gen = v.makeReg();
  v << loadb{rvmtl()[rds::currentGenNumberHandle()], gen};
  v << storeb{gen, rvmtl()[rds::genNumberHandleFrom(ch)]};
}

////////////////////////////////////////////////////////////////////////////////

}}
