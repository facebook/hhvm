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

#include "hphp/runtime/vm/jit/func-prologue.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/irgen-func-prologue.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/util/arch.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"
#include "hphp/util/growable-vector.h"
#include "hphp/util/immed.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

TransContext prologue_context(TransID transID,
                              TransKind kind,
                              const Func* func,
                              int initSpOffset,
                              Offset entry) {
  return TransContext(
    transID,
    kind,
    TransFlags{},
    SrcKey{func, entry, SrcKey::PrologueTag{}},
    FPInvOffset{func->numSlotsInFrame()},
    0,
    nullptr
  );
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

std::tuple<TransLoc, TCA, CodeCache::View>
genFuncPrologue(TransID transID, TransKind kind,
                Func* func, int argc, CodeCache& code, CGMeta& fixups,
                tc::CodeMetaLock* locker) {
  auto context = prologue_context(transID, kind, func, argc,
                                  func->getEntryForNumArgs(argc));

  tracing::Block _{
    "gen-prologue",
    [&] { return traceProps(context).add("argc", argc); }
  };

  IRUnit unit{context, std::make_unique<AnnotationData>()};
  irgen::IRGS env{unit, nullptr, 0, nullptr, true};

  irgen::emitFuncPrologue(env, argc, transID);
  irgen::sealUnit(env);

  printUnit(2, unit, "After initial prologue generation");

  auto vunit = irlower::lowerUnit(env.unit, CodeKind::Prologue);

  if (locker) locker->lock();
  SCOPE_EXIT { if (locker) locker->unlock(); };
  tc::assertOwnsCodeLock();
  tc::assertOwnsMetadataLock();

  auto codeView = code.view(kind);
  TCA mainOrig = codeView.main().frontier();

  // If we're close to a cache line boundary, just burn some space to
  // try to keep the func and its body on fewer total lines.
  align(codeView.main(), &fixups, Alignment::CacheLineRoundUp,
        AlignContext::Dead);

  tc::TransLocMaker maker(codeView);
  maker.markStart();

  emitVunit(*vunit, env.unit, codeView, fixups);
  return std::make_tuple(maker.markEnd().loc(), mainOrig, codeView);
}

TransLoc genFuncBodyDispatch(Func* func, const DVFuncletsVec& dvs,
                             TransKind kind, CodeCache& code,
                             tc::CodeMetaLock* locker) {
  auto context = prologue_context(kInvalidTransID, kind, func,
                                  func->numSlotsInFrame(), func->base());

  tracing::Block _{
    "gen-func-body-dispatch",
    [&] { return traceProps(context); }
  };

  IRUnit unit{context, std::make_unique<AnnotationData>()};
  irgen::IRGS env{unit, nullptr, 0, nullptr};

  irgen::emitFuncBodyDispatch(env, dvs);
  irgen::sealUnit(env);

  CGMeta fixups;
  auto vunit = irlower::lowerUnit(env.unit, CodeKind::Prologue);

  if (locker) locker->lock();
  SCOPE_EXIT { if (locker) locker->unlock(); };

  auto const& codeView = code.view(kind);
  tc::TransLocMaker maker{codeView};
  maker.markStart();

  emitVunit(*vunit, env.unit, codeView, fixups);

  auto const loc = maker.markEnd().loc();

  if (RuntimeOption::EvalPerfRelocate) {
    GrowableVector<IncomingBranch> ibs;
    tc::recordPerfRelocMap(loc.mainStart(), loc.mainEnd(),
                           loc.frozenCodeStart(), loc.frozenEnd(),
                           context.initSrcKey, 0, ibs, fixups);
  }
  fixups.process(nullptr);

  return loc;
}

///////////////////////////////////////////////////////////////////////////////

}}
