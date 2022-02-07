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

#include "hphp/runtime/base/header-kind.h"

#include "hphp/runtime/vm/jit/guard-type-profile.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

#include "hphp/runtime/base/runtime-option.h"

#include "hphp/util/arch.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/assertions.h"

namespace HPHP::jit {

///////////////////////////////////////////////////////////////////////////////

inline Vptr memTVTypePtr(SSATmp* ptr, Vloc loc) {
  assertx(ptr->isA(TPtr) || ptr->isA(TLval));
  if (ptr->isA(TLval)) return *loc.reg(tv_lval::type_idx);

  return loc.reg()[TVOFF(m_type)];
}

inline Vptr memTVValPtr(SSATmp* ptr, Vloc loc) {
  assertx(ptr->isA(TPtr) || ptr->isA(TLval));
  if (ptr->isA(TLval)) return *loc.reg(tv_lval::val_idx);

  return loc.reg()[TVOFF(m_data)];
}

inline void emitTestTVType(Vout& v, Vreg sf, Immed s0, Vreg s1) {
  v << testbi{s0, s1, sf};
}

inline void emitTestTVType(Vout& v, Vreg sf, Immed s0, Vptr s1) {
  v << testbim{s0, s1, sf};
}

inline void emitCmpTVType(Vout& v, Vreg sf, DataType s0, Vptr s1) {
  v << cmpbim{static_cast<data_type_t>(s0), s1, sf};
}

inline void emitCmpTVType(Vout& v, Vreg sf, DataType s0, Vreg s1) {
  v << cmpbi{static_cast<data_type_t>(s0), s1, sf};
}

inline Vreg emitGetTVType(Vout& v, Vreg s) {
  return s;
}

inline Vreg emitGetTVType(Vout& v, Vptr s) {
  auto const d = v.makeReg();
  v << loadb{s, d};
  return d;
}

inline Vreg emitGetTVTypeQuad(Vout& v, Vreg s) {
  auto const d = v.makeReg();
  v << movsbq{s, d};
  return d;
}

inline Vreg emitGetTVTypeQuad(Vout& v, Vptr s) {
  auto const d = v.makeReg();
  v << loadsbq{s, d};
  return d;
}

template<typename TLoc>
inline Vreg emitMaskTVType(Vout& v, Immed s0, TLoc s1) {
  auto const rtype = emitGetTVType(v, s1);
  auto const dst = v.makeReg();
  v << andbi{s0, rtype, dst, v.makeReg()};
  return dst;
}

template<typename TLoc>
inline ConditionCode emitIsTVTypeRefCounted(Vout& v, Vreg sf, TLoc s1) {
  if (RuntimeOption::EvalJitProfileGuardTypes) {
    emitProfileGuardType(v, TUncounted);
  }
  emitTestTVType(v, sf, kRefCountedBit, s1);
  return CC_NZ;
}

///////////////////////////////////////////////////////////////////////////////

inline Vreg emitCmpRefCount(Vout& v, Immed s0, Vreg s1) {
  auto const sf = v.makeReg();
  v << cmplim{s0, s1[FAST_REFCOUNT_OFFSET], sf};
  return sf;
}

inline Vreg emitCmpRefCount(Vout& v, Immed s0, Vptr m) {
  auto const sf = v.makeReg();
  v << cmplim{s0, m + FAST_REFCOUNT_OFFSET, sf};
  return sf;
}

inline void emitStoreRefCount(Vout& v, Immed s0, Vreg s1) {
  emitStoreRefCount(v, s0, *s1);
}

inline void emitStoreRefCount(Vout& v, Immed s0, Vptr m) {
  v << storeli{s0, m + FAST_REFCOUNT_OFFSET};
}

inline Vreg emitDecRefCount(Vout& v, Vreg s0) {
  auto const sf = v.makeReg();
  v << declm{s0[FAST_REFCOUNT_OFFSET], sf};
  return sf;
}

template<class Destroy>
void emitDecRefWork(Vout& v, Vout& vcold, Vreg data,
                    Destroy destroy, bool unlikelyDestroy,
                    Reason reason) {
  auto const sf = emitCmpRefCount(v, OneReference, data);

  ifThenElse(
    v, vcold, CC_E, sf,
    destroy,
    [&] (Vout& v) {
      // If it's not static, actually reduce the reference count.  This does
      // another branch using the same status flags from the cmplim above.
      ifThen(v, CC_NL, sf,
             [&] (Vout& v) { emitDecRef(v, data, reason); },
             tag_from_string("decref-is-static")
      );
    },
    unlikelyDestroy,
    tag_from_string("decref-is-one")
  );
}

///////////////////////////////////////////////////////////////////////////////

}
