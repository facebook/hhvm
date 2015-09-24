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

#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/runtime/vm/class.h"

#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/cpp-call.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/immed.h"
#include "hphp/util/low-ptr.h"
#include "hphp/util/ringbuffer.h"
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

Vreg zeroExtendIfBool(Vout& v, const SSATmp* src, Vreg reg) {
  if (!src->isA(TBool)) return reg;

  // Zero-extend the bool from a byte to a quad.
  auto extended = v.makeReg();
  v << movzbq{reg, extended};
  return extended;
}

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

void emitCall(Vout& v, CppCall target, RegSet args) {
  switch (target.kind()) {
    case CppCall::Kind::Direct:
      v << call{static_cast<TCA>(target.address()), args};
      return;
    case CppCall::Kind::Virtual:
      // Virtual call.  Load method's address from proper offset off of object in
      // rdi, using rax as scratch.
      v << load{*reg::rdi, reg::rax};
      v << callm{reg::rax[target.vtableOffset()], args};
      return;
    case CppCall::Kind::ArrayVirt: {
      auto const addr = reinterpret_cast<intptr_t>(target.arrayTable());
      v << loadzbl{reg::rdi[HeaderKindOffset], reg::eax};
      if (deltaFits(addr, sz::dword)) {
        v << callm{baseless(reg::rax * 8 + addr), args};
      } else {
        auto const base = v.makeReg();
        v << ldimmq{addr, base};
        v << callm{base[reg::rax * 8], args};
      }
      static_assert(sizeof(HeaderKind) == 1, "");
      return;
    }
    case CppCall::Kind::Destructor:
      // this movzbq is only needed because callers aren't required to
      // zero-extend the type.
      auto zextType = v.makeReg();
      v << movzbq{target.reg(), zextType};
      auto dtor_ptr = lookupDestructor(v, zextType);
      v << callm{dtor_ptr, args};
      return;
  }
  not_reached();
}

Vptr lookupDestructor(Vout& v, Vreg type) {
  auto const table = reinterpret_cast<intptr_t>(g_destructors);
  always_assert_flog(deltaFits(table, sz::dword),
    "Destructor function table is expected to be in the data "
    "segment, with addresses less than 2^31"
  );
  static_assert((KindOfString   >> kShiftDataTypeToDestrIndex == 1) &&
                (KindOfArray    >> kShiftDataTypeToDestrIndex == 2) &&
                (KindOfObject   >> kShiftDataTypeToDestrIndex == 3) &&
                (KindOfResource >> kShiftDataTypeToDestrIndex == 4) &&
                (KindOfRef      >> kShiftDataTypeToDestrIndex == 5),
                "lookup of destructors depends on KindOf* values");
  auto index = v.makeReg();
  v << shrli{kShiftDataTypeToDestrIndex, type, index, v.makeReg()};
  return baseless(index * 8 + safe_cast<int>(table));
}

///////////////////////////////////////////////////////////////////////////////

Vreg emitLdObjClass(Vout& v, Vreg obj, Vreg d) {
  emitLdLowPtr(v, obj[ObjectData::getVMClassOffset()], d,
               sizeof(LowPtr<Class>));
  return d;
}

Vreg emitLdClsCctx(Vout& v, Vreg src, Vreg dst) {
  v << decq{src, dst, v.makeReg()};
  return dst;
}

void emitCmpClass(Vout& v, Vreg sf, const Class* cls, Vptr mem) {
  auto size = sizeof(LowPtr<Class>);
  if (size == 8) {
    v << cmpqm{v.cns(cls), mem, sf};
  } else if (size == 4) {
    auto const clsImm = safe_cast<uint32_t>(reinterpret_cast<intptr_t>(cls));
    v << cmplm{v.cns(clsImm), mem, sf};
  } else {
    not_implemented();
  }
}

void emitCmpClass(Vout& v, Vreg sf, Vreg reg, Vptr mem) {
  auto size = sizeof(LowPtr<Class>);
  if (size == 8) {
    v << cmpqm{reg, mem, sf};
  } else if (size == 4) {
    auto lowCls = v.makeReg();
    v << movtql{reg, lowCls};
    v << cmplm{lowCls, mem, sf};
  } else {
    not_implemented();
  }
}

void emitCmpClass(Vout& v, Vreg sf, Vreg reg1, Vreg reg2) {
  auto size = sizeof(LowPtr<Class>);
  if (size == 8) {
    v << cmpq{reg1, reg2, sf};
  } else if (size == 4) {
    v << cmpl{reg1, reg2, sf};
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

void emitTransCounterInc(Vout& v) {
  if (!mcg->tx().isTransDBEnabled()) return;
  auto t = v.cns(mcg->tx().getTransCounterAddr());
  v << incqmlock{*t, v.makeReg()};
}

void emitRB(Vout& v, Trace::RingBufferType t, const char* msg) {
  if (!Trace::moduleEnabled(Trace::ringbuffer, 1)) return;
  v << vcall{CppCall::direct(Trace::ringbufferMsg),
             v.makeVcallArgs({{v.cns(msg), v.cns(strlen(msg)), v.cns(t)}}),
             v.makeTuple({})};
}

///////////////////////////////////////////////////////////////////////////////

}}
