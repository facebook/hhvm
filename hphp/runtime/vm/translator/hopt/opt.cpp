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

/*
 * Insert a DbgAssertRefCount instruction after each place we produce
 * a refcounted value.  The value must be something we can safely dereference
 * to check the _count field.
 */
static void insertRefCountAsserts(Trace* trace, IRFactory* factory) {
  forEachTraceBlock(trace, [&](Block* block) {
    for (auto it = block->begin(); it != block->end(); ) {
      IRInstruction& inst = *it;
      ++it;
      for (SSATmp& dst : inst.getDsts()) {
        Type t = dst.getType();
        if (t.subtypeOf(Type::Counted | Type::StaticStr | Type::StaticArr)) {
          auto* assertInst = factory->gen(DbgAssertRefCount, &dst);
          if (inst.isBlockEnd()) {
            // cannot insert after a branch.  A branch with a dest
            // is a guard, and the guard's result is only valid on the
            // fallthrough path.  so put the assert there.
            block->getNext()->prepend(assertInst);
          } else {
            block->insert(it, factory->gen(DbgAssertRefCount, &dst));
          }
        }
      }
    }
  });
}

void optimizeTrace(Trace* trace, TraceBuilder* traceBuilder) {
  IRFactory* irFactory = traceBuilder->getIrFactory();
  if (RuntimeOption::EvalHHIRMemOpt) {
    optimizeMemoryAccesses(trace, irFactory);
    if (RuntimeOption::EvalDumpIR > 5) {
      std::cout << "----- HHIR after MemElim -----\n";
      trace->print(std::cout, false);
      std::cout << "---------------------------\n";
    }
    assert(JIT::checkCfg(trace, *irFactory));
  }
  if (RuntimeOption::EvalHHIRDeadCodeElim) {
    eliminateDeadCode(trace, irFactory);
    if (RuntimeOption::EvalDumpIR > 5) {
      std::cout << "----- HHIR after DCE -----\n";
      trace->print(std::cout, false);
      std::cout << "---------------------------\n";
    }
    assert(JIT::checkCfg(trace, *irFactory));
  }
  if (RuntimeOption::EvalHHIRExtraOptPass
      && (RuntimeOption::EvalHHIRCse
          || RuntimeOption::EvalHHIRSimplification)) {
    traceBuilder->optimizeTrace();
    if (RuntimeOption::EvalDumpIR > 5) {
      std::cout << "----- HHIR after CSE/Simplification -----\n";
      trace->print(std::cout, false);
      std::cout << "---------------------------\n";
    }
    assert(JIT::checkCfg(trace, *irFactory));
    // Cleanup any dead code left around by CSE/Simplification
    // Ideally, this would be controlled by a flag returned
    // by optimzeTrace indicating whether DCE is necessary
    if (RuntimeOption::EvalHHIRDeadCodeElim) {
      eliminateDeadCode(trace, irFactory);
      if (RuntimeOption::EvalDumpIR > 5) {
        std::cout << "----- HHIR after DCE -----\n";
        trace->print(std::cout, false);
        std::cout << "---------------------------\n";
      }
      assert(JIT::checkCfg(trace, *irFactory));
    }
  }
  if (RuntimeOption::EvalHHIRJumpOpts) {
    optimizeJumps(trace, irFactory);
    if (RuntimeOption::EvalDumpIR > 5) {
      std::cout << "----- HHIR after jump opts -----\n";
      trace->print(std::cout, false);
      std::cout << "---------------------------\n";
    }
    assert(JIT::checkCfg(trace, *irFactory));
  }
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    insertRefCountAsserts(trace, irFactory);
    if (RuntimeOption::EvalDumpIR > 5) {
      std::cout << "----- HHIR after inserting RefCnt asserts -----\n";
      trace->print(std::cout, false);
      std::cout << "---------------------------\n";
    }
    assert(JIT::checkCfg(trace, *irFactory));
  }
}

} } }
