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
#include "dce.h"
#include "opt.h"
#include "memelim.h"
#include <util/trace.h>


namespace HPHP {
namespace VM {
namespace JIT {

void insertRefCountAssertsAux(Trace* trace, IRFactory* factory) {
  IRInstruction::List& instructions = trace->getInstructionList();
  IRInstruction::Iterator it;
  for (it = instructions.begin(); it != instructions.end(); ) {
    IRInstruction* inst = *it;
    it++;
    SSATmp* dst = inst->getDst();
    if (dst &&
        Type::isStaticallyKnown(dst->getType()) &&
        Type::isRefCounted(dst->getType())) {
      IRInstruction assertInst(AssertRefCount, Type::None, dst);
      auto toInsert = assertInst.clone(factory);
      toInsert->setParent(trace);
      instructions.insert(it, toInsert);
    }
  }
}

void insertRefCountAsserts(Trace* trace, IRFactory* factory) {
  insertRefCountAssertsAux(trace, factory);
  Trace::List& exitTraces = trace->getExitTraces();
  for (Trace::Iterator it = exitTraces.begin();
       it != exitTraces.end();
       it++) {
    insertRefCountAssertsAux(*it, factory);
  }
}

void optimizeTrace(Trace* trace, IRFactory* irFactory) {
  if (RuntimeOption::EvalHHIRMemOpt) {
    optimizeMemoryAccesses(trace, irFactory);
  }
  if (RuntimeOption::EvalDumpIR > 5) {
    std::cout << "----- HHIR before DCE -----\n";
    trace->print(std::cout, false);
    std::cout << "---------------------------\n";
  }
  eliminateDeadCode(trace, irFactory);
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    insertRefCountAsserts(trace, irFactory);
  }
}

} } }
