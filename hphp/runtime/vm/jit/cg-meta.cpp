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

void CGMeta::setJmpTransID(TCA jmp, TransID transID, TransKind kind) {
  if (kind != TransKind::Profile) return;

  FTRACE(5, "setJmpTransID: adding {} => {}\n", jmp, transID);
  jmpTransIDs.emplace_back(jmp, transID);
}

void CGMeta::process(
  GrowableVector<IncomingBranch>* inProgressTailBranches
) {
  process_only(inProgressTailBranches);
  clear();
}

void CGMeta::process_only(
  GrowableVector<IncomingBranch>* inProgressTailBranches
) {
  mcg->assertOwnsMetadataLock();

  for (auto const& pair : fixups) {
    assertx(mcg->code().isValidCodeAddress(pair.first));
    mcg->fixupMap().recordFixup(pair.first, pair.second);
  }
  fixups.clear();

  auto& ctm = mcg->catchTraceMap();
  for (auto const& pair : catches) {
    auto const key = mcg->code().toOffset(pair.first);
    auto const val = mcg->code().toOffset(pair.second);
    if (auto pos = ctm.find(key)) {
      *pos = val;
    } else {
      ctm.insert(key, val);
    }
  }
  catches.clear();

  if (auto profData = jit::profData()) {
    for (auto const& elm : jmpTransIDs) {
      profData->setJmpTransID(elm.first, elm.second);
    }
  }
  jmpTransIDs.clear();

  for (auto& pair : literals) {
    if (mcg->literals().find(pair.first)) continue;
    mcg->literals().insert(pair.first, pair.second);
  }
  literals.clear();

  if (inProgressTailBranches) {
    inProgressTailJumps.swap(*inProgressTailBranches);
  }
  assertx(inProgressTailJumps.empty());

  for (auto& stub : reusedStubs) {
    mcg->debugInfo()->recordRelocMap(stub, nullptr, "NewStub");
  }
  reusedStubs.clear();
}

void CGMeta::clear() {
  watchpoints.clear();
  fixups.clear();
  catches.clear();
  jmpTransIDs.clear();
  literals.clear();
  alignments.clear();
  reusedStubs.clear();
  addressImmediates.clear();
  codePointers.clear();
  inProgressTailJumps.clear();
  bcMap.clear();
}

bool CGMeta::empty() const {
  return
    watchpoints.empty() &&
    fixups.empty() &&
    catches.empty() &&
    jmpTransIDs.empty() &&
    literals.empty() &&
    alignments.empty() &&
    reusedStubs.empty() &&
    addressImmediates.empty() &&
    codePointers.empty() &&
    inProgressTailJumps.empty() &&
    bcMap.empty() &&
    true;
}

}}
