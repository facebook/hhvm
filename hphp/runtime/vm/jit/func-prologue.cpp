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

#include "hphp/runtime/vm/jit/func-prologue.h"

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/code-gen.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irgen-func-prologue.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/code-cache.h"
#include "hphp/util/data-block.h"
#include "hphp/util/immed.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

void genFuncGuard(Func* func, CodeBlock& cb) {
  Vauto vasm { cb };

  auto& v = vasm.main();
  auto const done = v.makeBlock();

  auto const sf = v.makeReg();

  auto const funcImm = Immed64(func);
  auto const arFunc = x64::rVmFp[AROFF(m_func)];

  if (funcImm.fits(sz::dword)) {
    v << cmpqims{funcImm.l(), arFunc, sf};
  } else {
    auto const rScratch = v.makeReg();
    v << ldimmqs{funcImm, rScratch};
    v << cmpqm{rScratch, arFunc, sf};
  }
  v << jcci{CC_NZ, sf, done, mcg->tx().uniqueStubs.funcPrologueRedispatch};

  v = done;
  v << fallthru{};
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

TCA genFuncPrologue(TransID transID, Func* func, int argc) {
  auto const context = TransContext(
    transID,
    SrcKey{func, func->getEntryForNumArgs(argc), SrcKey::PrologueTag{}},
    FPInvOffset{func->numSlotsInFrame()}
  );
  IRGS env{context, TransFlags{0}};

  auto& cb = mcg->code.main();

  // Dump the func guard in the TC before anything else.
  genFuncGuard(func, cb);
  auto start = cb.frontier();

  irgen::emitFuncPrologue(env, argc, transID);
  irgen::sealUnit(env);
  genCode(env.unit, CodeKind::CrossTrace);

  return start;
}

///////////////////////////////////////////////////////////////////////////////

}}
