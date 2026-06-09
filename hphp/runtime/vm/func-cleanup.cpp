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

#include "hphp/runtime/vm/func-cleanup.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/tc.h"

namespace HPHP {

namespace {

std::mutex s_dataLock;

// Keyed on FuncId as these are never reused
hphp_fast_map<const Func*, FuncCleanup::Metadata, pointer_hash<Func>> s_funcExtraMetadata;

}

FuncCleanup::Metadata& FuncCleanup::metadata() {
  return s_funcExtraMetadata[m_func];
}

std::unique_ptr<FuncCleanup> FuncCleanup::get(const Func* func) {
  if (func->isPersistent()) return nullptr;
  return std::make_unique<FuncCleanup>(func, std::unique_lock<std::mutex>(s_dataLock));
}

std::unique_ptr<FuncCleanup> FuncCleanup::get(FuncId funcId) {
  if (funcId.isInvalid()) return nullptr;
  auto func = Func::fromFuncId(funcId);
  return get(func);
}

void FuncCleanup::cleanup() {
  assertx(!m_func->isPersistent());
  auto it = s_funcExtraMetadata.find(m_func);
  if (it == s_funcExtraMetadata.end()) return;

  if (auto pd = jit::globalProfData()) {
    const_cast<jit::ProfData*>(pd)->cleanupFunc(m_func,
                                                it->second.profDataSks,
                                                it->second.profDataPrologueIDs);
  }

  jit::tc::cleanupSrcDBKeys(it->second.srcKeys);

  auto recycleInfo = std::move(it->second.recycleInfo);
  s_funcExtraMetadata.erase(it);

  if (jit::mcgen::initialized() && Cfg::Eval::EnableReusableTC) {
    // Free TC-space associated with func
    jit::tc::reclaimFunction(m_func->name(), std::move(recycleInfo));
  }
}

void FuncCleanup::addSrcDBKey(const SrcKey sk) {
  auto fc = FuncCleanup::get(sk.func());
  if (!fc) return;
  auto& m = fc->metadata();
  m.srcKeys.emplace_back(sk);
}

void FuncCleanup::addProfDataSks(const SrcKey sk) {
  auto fc = FuncCleanup::get(sk.func());
  if (!fc) return;
  auto& m = fc->metadata();
  m.profDataSks.emplace_back(sk);
}

void FuncCleanup::addProfDataPrologueID(const PrologueID id) {
  auto fc = FuncCleanup::get(id.func());
  if (!fc) return;
  auto& m = fc->metadata();
  m.profDataPrologueIDs.emplace_back(id);
}

} // namespace HPHP
