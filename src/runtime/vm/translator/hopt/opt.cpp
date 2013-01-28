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

static void insertRefCountAssertsAux(Trace* trace, IRFactory* factory) {
  IRInstruction::List& instructions = trace->getInstructionList();
  IRInstruction::Iterator it;
  for (it = instructions.begin(); it != instructions.end(); ) {
    IRInstruction* inst = *it;
    it++;
    SSATmp* dst = inst->getDst();
    if (dst &&
        Type::isStaticallyKnown(dst->getType()) &&
        Type::isRefCounted(dst->getType())) {
      auto* assertInst = factory->gen(DbgAssertRefCount, dst);
      assertInst->setParent(trace);
      instructions.insert(it, assertInst);
    }
  }
}

static void insertRefCountAsserts(Trace* trace, IRFactory* factory) {
  insertRefCountAssertsAux(trace, factory);
  for (Trace* exit : trace->getExitTraces()) {
    insertRefCountAssertsAux(exit, factory);
  }
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
  }
  if (RuntimeOption::EvalHHIRDeadCodeElim) {
    eliminateDeadCode(trace, irFactory);
    if (RuntimeOption::EvalDumpIR > 5) {
      std::cout << "----- HHIR after DCE -----\n";
      trace->print(std::cout, false);
      std::cout << "---------------------------\n";
    }
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
    }
  }
  if (RuntimeOption::EvalHHIRJumpOpts) {
    optimizeJumps(trace, irFactory);
    if (RuntimeOption::EvalDumpIR > 5) {
      std::cout << "----- HHIR after jump opts -----\n";
      trace->print(std::cout, false);
      std::cout << "---------------------------\n";
    }
  }
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    insertRefCountAsserts(trace, irFactory);
  }
}

} } }
