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

#include "hphp/runtime/vm/jit/cg-meta.h"

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/prof-data.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(mcg);

void CGMeta::setJmpTransID(TCA jmp, TransKind kind) {
  if (kind != TransKind::Profile) return;

  TransID transId = mcg->tx().profData()->curTransID();
  FTRACE(5, "setJmpTransID: adding {} => {}\n", jmp, transId);
  jmpTransIDs.emplace_back(jmp, transId);
}

void CGMeta::process(
  GrowableVector<IncomingBranch>* inProgressTailBranches
) {
  process_only(inProgressTailBranches);
  clear();
}

void
CGMeta::process_only(
  GrowableVector<IncomingBranch>* inProgressTailBranches) {
  for (auto const& pair : fixups) {
    assertx(mcg->code().isValidCodeAddress(pair.first));
    mcg->fixupMap().recordFixup(pair.first, pair.second);
  }
  fixups.clear();

  auto& ctm = mcg->catchTraceMap();
  for (auto const& pair : catches) {
    if (auto pos = ctm.find(pair.first)) {
      *pos = pair.second;
    } else {
      ctm.insert(pair.first, pair.second);
    }
  }
  catches.clear();

  for (auto const& elm : jmpTransIDs) {
    mcg->getJmpToTransIDMap()[elm.first] = elm.second;
  }
  jmpTransIDs.clear();

  mcg->literals().insert(literals.begin(), literals.end());
  literals.clear();

  if (inProgressTailBranches) {
    inProgressTailJumps.swap(*inProgressTailBranches);
  }
  assertx(inProgressTailJumps.empty());

  for (auto& stub : reusedStubs) {
    mcg->getDebugInfo()->recordRelocMap(stub, nullptr, "NewStub");
  }
  reusedStubs.clear();
}

void CGMeta::clear() {
  fixups.clear();
  catches.clear();
  jmpTransIDs.clear();
  reusedStubs.clear();
  addressImmediates.clear();
  codePointers.clear();
  bcMap.clear();
  alignments.clear();
  inProgressTailJumps.clear();
  literals.clear();
  annotations.clear();
}

bool CGMeta::empty() const {
  return
    fixups.empty() &&
    catches.empty() &&
    jmpTransIDs.empty() &&
    reusedStubs.empty() &&
    addressImmediates.empty() &&
    codePointers.empty() &&
    bcMap.empty() &&
    alignments.empty() &&
    inProgressTailJumps.empty() &&
    literals.empty() &&
    annotations.empty();
}

}}
