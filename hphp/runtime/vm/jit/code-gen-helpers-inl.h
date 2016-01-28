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

#include "hphp/runtime/base/header-kind.h"

#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

#include "hphp/util/asm-x64.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

inline void emitLoadTVType(Vout& v, Vptr mem, Vreg8 d) {
  v << loadb{mem, d};
}

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

///////////////////////////////////////////////////////////////////////////////

template<class Destroy>
void emitDecRefWork(Vout& v, Vout& vcold, Vreg data,
                    Destroy destroy, bool unlikelyDestroy) {
  auto const sf = v.makeReg();
  v << cmplim{1, data[FAST_REFCOUNT_OFFSET], sf};
  ifThenElse(v, vcold, CC_E, sf,
    destroy,
    [&] (Vout& v) {
      // If it's not static, actually reduce the reference count.  This does
      // another branch using the same status flags from the cmplim above.
      ifThen(v, CC_NL, sf, [&] (Vout& v) { emitDecRef(v, data); });
    },
    unlikelyDestroy
  );
}

///////////////////////////////////////////////////////////////////////////////

}}
