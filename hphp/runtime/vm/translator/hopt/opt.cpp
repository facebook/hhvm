/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include "runtime/vm/translator/hopt/opt.h"
#include "runtime/vm/translator/hopt/tracebuilder.h"
#include "util/trace.h"
#include "runtime/vm/translator/hopt/irfactory.h"

namespace HPHP {
namespace VM {
namespace JIT {

// insert inst after the point dst is defined
static void insertAfter(IRInstruction* definer, IRInstruction* inst) {
  assert(!definer->isBlockEnd());
  Block* block = definer->getBlock();
  auto pos = block->iteratorTo(definer); ++pos;
  block->insert(pos, inst);
}

/*
 * Insert a DbgAssertRefCount instruction after each place we produce
 * a refcounted value.  The value must be something we can safely dereference
 * to check the _count field.
 */
static void insertRefCountAsserts(IRInstruction& inst, IRFactory* factory) {
  for (SSATmp& dst : inst.getDsts()) {
    Type t = dst.type();
    if (t.subtypeOf(Type::Counted | Type::StaticStr | Type::StaticArr)) {
      insertAfter(&inst, factory->gen(DbgAssertRefCount, &dst));
    }
  }
}

/*
 * Insert a DbgAssertTv instruction for each stack location stored to by
 * a SpillStack instruction.
 */
static void insertSpillStackAsserts(IRInstruction& inst, IRFactory* factory) {
  SSATmp* sp = inst.getDst();
  auto const vals = inst.getSrcs().subpiece(2);
  auto* block = inst.getBlock();
  auto pos = block->iteratorTo(&inst); ++pos;
  for (unsigned i = 0, n = vals.size(); i < n; ++i) {
    Type t = vals[i]->type();
    if (t.subtypeOf(Type::Gen)) {
      IRInstruction* addr = factory->gen(LdStackAddr, Type::PtrToGen,
                                         sp, factory->defConst(i));
      block->insert(pos, addr);
      IRInstruction* check = factory->gen(DbgAssertPtr, addr->getDst());
      block->insert(pos, check);
    }
  }
}

/*
 * Insert asserts at various points in the IR.
 * TODO: t2137231 Insert DbgAssertPtr at points that use or produces a GenPtr
 */
static void insertAsserts(Trace* trace, IRFactory* factory) {
  forEachTraceBlock(trace, [=](Block* block) {
    for (auto it = block->begin(), end = block->end(); it != end; ) {
      IRInstruction& inst = *it;
      ++it;
      if (inst.op() == SpillStack) {
        insertSpillStackAsserts(inst, factory);
        continue;
      }
      if (inst.op() == Call) {
        SSATmp* sp = inst.getDst();
        IRInstruction* addr = factory->gen(LdStackAddr, Type::PtrToGen,
                                           sp, factory->defConst(0));
        insertAfter(&inst, addr);
        insertAfter(addr, factory->gen(DbgAssertPtr, addr->getDst()));
        continue;
      }
      if (!inst.isBlockEnd()) insertRefCountAsserts(inst, factory);
    }
  });
}

void optimizeTrace(Trace* trace, TraceBuilder* traceBuilder) {
  IRFactory* irFactory = traceBuilder->getIrFactory();
  auto finishPass = [&](const char* msg) {
    dumpTrace(6, trace, msg);
    assert(JIT::checkCfg(trace, *irFactory));
  };
  if (RuntimeOption::EvalHHIRMemOpt) {
    optimizeMemoryAccesses(trace, irFactory);
    finishPass("after MemeLim");
  }

  if (RuntimeOption::EvalHHIRDeadCodeElim) {
    eliminateDeadCode(trace, irFactory);
    finishPass("after DCE");
  }

  if (RuntimeOption::EvalHHIRExtraOptPass
      && (RuntimeOption::EvalHHIRCse
          || RuntimeOption::EvalHHIRSimplification)) {
    traceBuilder->optimizeTrace();
    finishPass("after CSE/Simplification");
    // Cleanup any dead code left around by CSE/Simplification
    // Ideally, this would be controlled by a flag returned
    // by optimzeTrace indicating whether DCE is necessary
    if (RuntimeOption::EvalHHIRDeadCodeElim) {
      eliminateDeadCode(trace, irFactory);
      finishPass("after DCE");
    }
  }

  if (RuntimeOption::EvalHHIRJumpOpts) {
    optimizeJumps(trace, irFactory);
    finishPass("jump opts");
  }

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    insertAsserts(trace, irFactory);
    finishPass("RefCnt asserts");
  }
}

} } }
