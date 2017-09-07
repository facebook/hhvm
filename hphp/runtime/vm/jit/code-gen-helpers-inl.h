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

#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

#include "hphp/util/arch.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/assertions.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

inline void emitTestTVType(Vout& v, Vreg sf, Immed s0, Vreg s1) {
  v << testbi{s0, s1, sf};
}

inline void emitTestTVType(Vout& v, Vreg sf, Immed s0, Vptr s1) {
  v << testbim{s0, s1, sf};
}

inline void emitCmpTVType(Vout& v, Vreg sf, Immed s0, Vptr s1) {
  v << cmpbim{s0, s1, sf};
}

inline void emitCmpTVType(Vout& v, Vreg sf, Immed s0, Vreg s1) {
  v << cmpbi{s0, s1, sf};
}

inline Vreg emitMaskTVType(Vout& v, Immed s0, Vreg s1) {
  auto const dst = v.makeReg();
  v << andbi{s0, s1, dst, v.makeReg()};
  return dst;
}

inline Vreg emitMaskTVType(Vout& v, Immed s0, Vptr s1) {
  auto const reg = v.makeReg();
  v << loadb{s1, reg};
  return emitMaskTVType(v, s0, reg);
}

///////////////////////////////////////////////////////////////////////////////

inline Vreg emitCmpRefCount(Vout& v, Immed s0, Vreg s1) {
  auto const sf = v.makeReg();

  if (one_bit_refcount) {
    v << cmpbim{s0, s1[FAST_REFCOUNT_OFFSET], sf};
  } else {
    v << cmplim{s0, s1[FAST_REFCOUNT_OFFSET], sf};
  }

  return sf;
}

inline void emitStoreRefCount(Vout& v, Immed s0, Vreg s1) {
  emitStoreRefCount(v, s0, *s1);
}

inline void emitStoreRefCount(Vout& v, Immed s0, Vptr m) {
  if (one_bit_refcount) {
    v << storebi{s0, m + FAST_REFCOUNT_OFFSET};
  } else {
    v << storeli{s0, m + FAST_REFCOUNT_OFFSET};
  }
}

inline Vreg emitDecRefCount(Vout& v, Vreg s0) {
  always_assert(
    !one_bit_refcount &&
    "Reference counts should never be decremented in one-bit mode"
  );

  auto const sf = v.makeReg();
  v << declm{s0[FAST_REFCOUNT_OFFSET], sf};
  return sf;
}

template<class Destroy>
void emitDecRefWork(Vout& v, Vout& vcold, Vreg data,
                    Destroy destroy, bool unlikelyDestroy) {
  auto const sf = emitCmpRefCount(v, OneReference, data);

  if (one_bit_refcount) {
    ifThen(
      v, vcold, CC_E, sf, destroy, unlikelyDestroy,
      tag_from_string("decref-is-one")
    );
  } else {
    ifThenElse(
      v, vcold, CC_E, sf,
      destroy,
      [&] (Vout& v) {
        // If it's not static, actually reduce the reference count.  This does
        // another branch using the same status flags from the cmplim above.
        ifThen(v, CC_NL, sf, [&] (Vout& v) { emitDecRef(v, data); },
               tag_from_string("decref-is-static"));
      },
      unlikelyDestroy,
      tag_from_string("decref-is-one")
    );
  }
}

///////////////////////////////////////////////////////////////////////////////

}}
