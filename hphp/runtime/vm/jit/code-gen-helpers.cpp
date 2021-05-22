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
#include "hphp/runtime/vm/jit/code-gen-tls.h"

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
#include "hphp/runtime/vm/jit/translator-inline.h"
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

void assertSFNonNegative(Vout& v, Vreg sf, Reason reason) {
  if (!RuntimeOption::EvalHHIRGenerateAsserts) return;
  ifThen(v, CC_NGE, sf, [&] (Vout& v) { v << trap{reason}; });
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

namespace {

void storeTVVal(Vout& v, Type type, Vloc srcLoc, Vptr valPtr) {
  // We ignore the values of statically nullish types.
  if (type <= TNull || type <= TNullptr) return;

  // Store the value.
  if (type.hasConstVal()) {
    // Skip potential zero-extend if we know the value.
    v << store{v.cns(type.rawVal()), valPtr};
  } else {
    assertx(srcLoc.hasReg(0));
    auto const extended = zeroExtendIfBool(v, type, srcLoc.reg(0));
    v << store{extended, valPtr};
  }
}

}

void storeTV(Vout& v, Vptr dst, Vloc srcLoc, const SSATmp* src, Type ty) {
  if (ty == TBottom) ty = src->type();
  storeTV(v, ty, srcLoc, dst + TVOFF(m_type), dst + TVOFF(m_data));
}

void storeTV(Vout& v, Type type, Vloc srcLoc, Vptr typePtr, Vptr valPtr) {
  if (srcLoc.isFullSIMD()) {
    // The whole TV is stored in a single SIMD reg.
    assertx(RuntimeOption::EvalHHIRAllocSIMDRegs);
    always_assert(typePtr == valPtr + (TVOFF(m_type) - TVOFF(m_data)));
    v << storeups{srcLoc.reg(), valPtr};
    return;
  }

  if (type.needsReg()) {
    assertx(srcLoc.hasReg(1));
    v << storeb{srcLoc.reg(1), typePtr};
  } else {
    v << storeb{v.cns(type.toDataType()), typePtr};
  }

  storeTVVal(v, type, srcLoc, valPtr);
}

void storeTVWithAux(Vout& v,
                    Vptr dst,
                    Vloc srcLoc,
                    const SSATmp* src,
                    AuxUnion aux) {
  static_assert(TVOFF(m_type) == 8, "");
  static_assert(TVOFF(m_aux) == 12, "");
  assertx(!srcLoc.isFullSIMD());

  auto const type = src->type();
  auto const auxMask = auxToMask(aux);

  if (type.needsReg()) {
    assertx(srcLoc.hasReg(1));

    // DataType is signed. We're using movzbq here to clear out the upper 7
    // bytes of the register, not to actually extend the type value.
    auto const typeReg = srcLoc.reg(1);
    auto const extended = v.makeReg();
    auto const result = v.makeReg();
    v << movzbq{typeReg, extended};
    v << orq{extended, v.cns(auxMask), result, v.makeReg()};
    v << store{result, dst + TVOFF(m_type)};
  } else {
    auto const dt = static_cast<std::make_unsigned<data_type_t>::type>(
      type.toDataType()
    );
    static_assert(std::numeric_limits<decltype(dt)>::digits <= 32, "");
    v << store{v.cns(dt | auxMask), dst + TVOFF(m_type)};
  }

  storeTVVal(v, type, srcLoc, dst + TVOFF(m_data));
}

void loadTV(Vout& v, const SSATmp* dst, Vloc dstLoc, Vptr src,
            bool aux /* = false */) {
  loadTV(v, dst->type(), dstLoc, src + TVOFF(m_type), src + TVOFF(m_data), aux);
}

void loadTV(Vout& v, Type type, Vloc dstLoc, Vptr typePtr, Vptr valPtr,
            bool aux) {
  if (dstLoc.isFullSIMD()) {
    // The whole TV is loaded into a single SIMD reg.
    assertx(RuntimeOption::EvalHHIRAllocSIMDRegs);
    always_assert(typePtr == valPtr + (TVOFF(m_type) - TVOFF(m_data)));
    v << loadups{valPtr, dstLoc.reg()};
    return;
  }

  if (type.needsReg() || aux) {
    assertx(dstLoc.hasReg(1));
    if (aux) {
      v << load{typePtr, dstLoc.reg(1)};
    } else {
      v << loadb{typePtr, dstLoc.reg(1)};
    }
  }

  if (type <= TBool) {
    v << loadtqb{valPtr, dstLoc.reg(0)};
  } else {
    v << load{valPtr, dstLoc.reg(0)};
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
    v << copyargs{
      v.makeTuple({src.reg(0), src.reg(1)}),
      v.makeTuple({dst.reg(0), dst.reg(1)})
    };
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

void trashFullTV(Vout& v, Vptr ptr, char byte) {
  int32_t trash32;
  memset(&trash32, byte, sizeof(trash32));
  static_assert(sizeof(TypedValue) % sizeof(trash32) == 0, "");

  for (int offset = 0; offset < sizeof(TypedValue);
       offset += sizeof(trash32)) {
    v << storeli{trash32, ptr + offset};
  }
}

void trashTV(Vout& v, Vptr typePtr, Vptr valPtr, char byte) {
  int32_t trash32;
  memset(&trash32, byte, sizeof(trash32));
  static_assert(sizeof(Value) == 8, "");
  v << storeli{trash32, valPtr};
  v << storeli{trash32, valPtr + 4};

  static_assert(sizeof(DataType) == 1, "");
  v << storebi{byte, typePtr};
}

void emitAssertRefCount(Vout& v, Vreg data, Reason reason) {
  auto const sf = emitCmpRefCount(v, StaticValue, data);

  ifThen(v, CC_NLE, sf, [&] (Vout& v) {
    auto const sf = emitCmpRefCount(v, RefCountMaxRealistic, data);
    ifThen(v, CC_NBE, sf, [&] (Vout& v) { v << trap{reason}; });
  });
}

void emitIncRef(Vout& v, Vreg base, Reason reason) {
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitAssertRefCount(v, base, reason);
  }

  auto const sf = v.makeReg();
  v << inclm{base[FAST_REFCOUNT_OFFSET], sf};
  assertSFNonNegative(v, sf, reason);
}

Vreg emitDecRef(Vout& v, Vreg base, Reason reason) {
  auto const sf = emitDecRefCount(v, base);
  assertSFNonNegative(v, sf, reason);
  return sf;
}

void emitIncRefWork(Vout& v, Vreg data, Vreg type, Reason reason) {
  auto const sf = v.makeReg();
  auto const cc = emitIsTVTypeRefCounted(v, sf, type);
  // ifRefCountedType
  ifThen(v, cc, sf, [&] (Vout& v) {
    // One-bit mode: do the IncRef if m_count == OneReference (0). Normal mode:
    // do the IncRef if m_count >= 0.
    auto const sf2 = emitCmpRefCount(v, 0, data);
    auto const cc = CC_GE;
    ifThen(v, cc, sf2, [&] (Vout& v) { emitIncRef(v, data, reason); });
  });
}

void emitIncRefWork(Vout& v, Vloc loc, Type type, Reason reason) {
  // If definitely not ref-counted, nothing to do
  if (!type.maybe(TCounted)) return;

  if (type <= TCounted) {
    // Definitely ref-counted
    emitIncRef(v, loc.reg(), reason);
    return;
  }

  // It might be ref-counted, we need to check at runtime.

  if (loc.hasReg(1)) {
    // We don't know the type, so check it at runtime.
    emitIncRefWork(v, loc.reg(0), loc.reg(1), reason);
    return;
  }

  // We do know the type, but it might be persistent or counted. Check the
  // ref-count.
  auto const sf = emitCmpRefCount(v, 0, loc.reg());
  auto const cc = CC_GE;
  ifThen(v, cc, sf, [&] (Vout& v) { emitIncRef(v, loc.reg(), reason); });
}

void emitDecRefWorkObj(Vout& v, Vreg obj, Reason reason) {
  auto const shouldRelease = emitCmpRefCount(v, OneReference, obj);
  ifThenElse(
    v, CC_E, shouldRelease,
    [&] (Vout& v) {
      // Put fn inside vcall{} triggers a compiler internal error (gcc 4.4.7)
      auto const cls = emitLdObjClass(v, obj, v.makeReg());
      auto const fn = CallSpec::objDestruct(cls);
      v << vcall{fn, v.makeVcallArgs({{obj, cls}}), v.makeTuple({}),
                 Fixup::none()};
    },
    [&] (Vout& v) {
      emitDecRef(v, obj, reason);
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

    case K::Destructor: {
      auto dtor = lookupDestructor(v, target.reg());
      v << callm{dtor, args};
    } return;

    case K::ObjDestructor: {
      auto const func = v.makeReg();
      emitLdLowPtr(
        v,
        target.reg()[Class::releaseFuncOff()],
        func,
        sizeof(ObjReleaseFunc)
      );
      v << callr{func, args};
    } return;

    case K::Stub:
      v << callstub{target.stubAddr(), args};
      return;
  }
  not_reached();
}

Vptr lookupDestructor(Vout& v, Vreg type, bool typeIsQuad) {
  auto const elem_sz = static_cast<int>(sizeof(g_destructors[0]) / 2);
  auto const table = reinterpret_cast<intptr_t>(&g_destructors[0]) -
    kMinRefCountedDataType * elem_sz;

  auto const index = [&] {
    if (typeIsQuad) return type;
    auto const r = v.makeReg();
    v << movsbq{type, r};
    return r;
  }();

  // The baseless form is more compact, but isn't supported for 64-bit
  // displacements.
  if (table <= std::numeric_limits<int>::max()) {
    return baseless(index * elem_sz + safe_cast<int>(table));
  }
  return v.cns(table)[index * elem_sz];
}

///////////////////////////////////////////////////////////////////////////////
Vreg emitLdRecDesc(Vout& v, Vreg val, Vreg d) {
  emitLdLowPtr(v, val[RecordData::getVMRecordOffset()], d,
               sizeof(LowPtr<RecordDesc>));
  return d;
}

Vreg emitLdObjClass(Vout& v, Vreg obj, Vreg d) {
  emitLdLowPtr(v, obj[ObjectData::getVMClassOffset()], d,
               sizeof(LowPtr<Class>));
  return d;
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

/*
 * Generate range check for isCollection:
 * set CC_BE if obj->m_kind - HeaderKind::Vector <= HeaderKind::ImmSet
 */
Vreg emitIsCollection(Vout& v, Vreg obj) {
  auto const sf = v.makeReg();
  auto const mincol = static_cast<int>(HeaderKind::Vector);
  auto const maxcol = static_cast<int>(HeaderKind::ImmSet);
  auto const kind = v.makeReg();
  auto const col_kind = v.makeReg();
  v << loadzbl{obj[HeaderKindOffset], kind};
  v << subli{mincol, kind, col_kind, v.makeReg()};
  v << cmpli{maxcol - mincol, col_kind, sf};
  return sf;
}

///////////////////////////////////////////////////////////////////////////////

static std::atomic<int32_t> s_nextFakeAddress{-1};

void emitEagerSyncPoint(Vout& v, PC pc, Vreg rds, Vreg vmfp, Vreg vmsp) {
  v << store{vmfp, rds[rds::kVmfpOff]};
  v << store{vmsp, rds[rds::kVmspOff]};
  emitImmStoreq(v, intptr_t(pc), rds[rds::kVmpcOff]);

  auto const addr = s_nextFakeAddress.fetch_sub(1, std::memory_order_relaxed);
  v << storeqi{addr, rds[rds::kVmJitReturnAddrOff]};
  v << recordstack{(TCA)static_cast<int64_t>(addr)};
}

void emitRB(Vout& v, Trace::RingBufferType t, const char* msg) {
  if (!Trace::moduleEnabled(Trace::ringbuffer, 1)) return;
  v << vcall{CallSpec::direct(Trace::ringbufferMsg),
             v.makeVcallArgs({{v.cns(msg), v.cns(strlen(msg)), v.cns(t)}}),
             v.makeTuple({}),
             Fixup::none()};
}

void emitIncStat(Vout& v, Stats::StatCounter stat) {
  if (!Stats::enabled()) return;
  auto rdslocalBase = v.makeReg();
  auto datum = tls_datum(rds::local::detail::rl_hotSection.rdslocal_base);
  auto offset = Stats::rl_counters.getRawOffset() +
                offsetof(Stats::StatCounters, counters) +
                sizeof(decltype(stat))*stat;
  v << load{emitTLSAddr(v, datum), rdslocalBase};
  v << incqm{rdslocalBase[offset], v.makeReg()};
}

///////////////////////////////////////////////////////////////////////////////

static Vptr getRDSHandleGenNumberAddr(rds::Handle handle) {
  return rvmtl()[rds::genNumberHandleFrom(handle)];
}

static Vptr getRDSHandleGenNumberAddr(Vreg handle) {
  return handle[DispReg(rvmtl(), -sizeof(rds::GenNumber))];
}

template<typename HandleT>
Vreg doCheckRDSHandleInitialized(Vout& v, HandleT ch) {
  auto const gen = v.makeReg();
  auto const sf = v.makeReg();
  v << loadb{getRDSHandleGenNumberAddr(ch), gen};
  v << cmpbm{gen, rvmtl()[rds::currentGenNumberHandle()], sf};
  return sf;
}

Vreg checkRDSHandleInitialized(Vout& v, rds::Handle ch) {
  assertx(rds::isNormalHandle(ch));
  return doCheckRDSHandleInitialized(v, ch);
}
Vreg checkRDSHandleInitialized(Vout& v, Vreg ch) {
  return doCheckRDSHandleInitialized(v, ch);
}

template<typename HandleT>
void doMarkRDSHandleInitialized(Vout &v, HandleT ch) {
  auto const gen = v.makeReg();
  v << loadb{rvmtl()[rds::currentGenNumberHandle()], gen};
  v << storeb{gen, getRDSHandleGenNumberAddr(ch)};
}

void markRDSHandleInitialized(Vout& v, rds::Handle ch) {
  assertx(rds::isNormalHandle(ch));
  doMarkRDSHandleInitialized(v, ch);
}

void markRDSHandleInitialized(Vout& v, Vreg ch) {
  doMarkRDSHandleInitialized(v, ch);
}

////////////////////////////////////////////////////////////////////////////////

int offsetToLocalType(int id) {
  return TVOFF(m_type) - cellsToBytes(id + 1);
}
int offsetToLocalData(int id) {
  return TVOFF(m_data) - cellsToBytes(id + 1);
}

Vptr ptrToLocalType(Vreg fp, int id) {
  return fp[offsetToLocalType(id)];
}
Vptr ptrToLocalData(Vreg fp, int id) {
  return fp[offsetToLocalData(id)];
}

void nextLocal(Vout& v,
               Vreg typeIn,
               Vreg dataIn,
               Vreg typeOut,
               Vreg dataOut,
               unsigned distance) {
  v << subqi{(int32_t)(sizeof(TypedValue) * distance), typeIn, typeOut,
      v.makeReg()};
  v << subqi{(int32_t)(sizeof(TypedValue) * distance), dataIn, dataOut,
      v.makeReg()};
}

void prevLocal(Vout& v,
               Vreg typeIn,
               Vreg dataIn,
               Vreg typeOut,
               Vreg dataOut) {
  v << addqi{(int32_t)sizeof(TypedValue), typeIn, typeOut, v.makeReg()};
  v << addqi{(int32_t)sizeof(TypedValue), dataIn, dataOut, v.makeReg()};
}

////////////////////////////////////////////////////////////////////////////////

uint64_t auxToMask(AuxUnion aux) {
  if (!aux.u_raw) return 0;
  if (aux.u_raw == static_cast<uint32_t>(-1)) {
    return static_cast<uint64_t>(-1) <<
      std::numeric_limits<
        std::make_unsigned<data_type_t>::type
      >::digits;
  }
  return uint64_t{aux.u_raw} << 32;
}

////////////////////////////////////////////////////////////////////////////////

}}
