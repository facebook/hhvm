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

#include "hphp/runtime/vm/jit/translation-stats.h"

#include <sstream>

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>


namespace HPHP::jit {

TransStatsRec::TransStatsRec(TransKind kind, SrcKey sk): m_kind(kind), m_sk(sk) {}

TransStats::TransStats()
  : m_counters(std::numeric_limits<int64_t>::max())
  , m_srcKeyToTransID(Cfg::Eval::FuncCountHint)
{ }

namespace {

uint64_t toAtomicKey(TransKind kind, SrcKey sk) {
  /* This is a bit tricky. We're encoding the kind (3 bits) in
    with the regular SrcKey to allow distinguishing between
    live/optimized/profiling translations of the same SK for
    stats reporting purposes. This is helpful when, for instance,
    a live translation writes over a optimized one due to a bug.

    The structure is:
        bits 32-63: func id
        bits 29-31: trans kind
        bits 3-28: argc/offset (3 fewer than what SrcKey allocates)
        bits 0-2: resume mode/prologue/FE tag

    Specifically, we're hijacking SrcKey's atomic int representation and
    relying on not having greater than than 2^26 offset or argc.
  */
  uint64_t key = sk.toAtomicInt();
  uint64_t transKindBits = static_cast<uint64_t>(kind);
  assertx(transKindBits < (1 << 3));
  // Ensure that the values we're writing over were always zero.
  uint64_t DEBUG_ONLY keyLower32Bits = key & 0xFFFFFFFFu;
  assertx((keyLower32Bits & (((1 << 3) - 1) << 28)) == 0);

  return key | (transKindBits << 28);
}
}

TransID TransStats::getTransID(TransKind kind, SrcKey sk) const {
  auto atomicKey = toAtomicKey(kind, sk);
  return folly::get_default(
    m_srcKeyToTransID,
    atomicKey,
    kInvalidTransID
  );
}


TransID TransStats::initTransStats(TransKind kind, SrcKey sk) {
  std::unique_lock lock{m_transLock};

  auto existingTransID = getTransID(kind, sk);
  if (existingTransID != kInvalidTransID) {
    return existingTransID;
  }
  m_transStatsRecs.emplace_back();
  TransID transID = m_transStatsRecs.size() - 1;
  m_transStatsRecs[transID].reset(new TransStatsRec(kind, sk));
  m_srcKeyToTransID.emplace(toAtomicKey(kind, sk), transID);
  return transID;
}

void TransStats::logCallCounts() const {
  auto const uuid = boost::uuids::random_generator()();
  auto const uuidStr = boost::uuids::to_string(uuid);

  for (TransID i = 0; i < m_transStatsRecs.size(); ++i) {
    const auto& rec = m_transStatsRecs[i];
    auto callCount = transStatsCounter(i);
    if (!folly::Random::oneIn(Cfg::Jit::TranslationStatsSampleRate)) {
      continue;
    }
    if (callCount >= Cfg::Jit::ReportTranslationStatsMinCallCount) {
      auto func = rec->srcKey().func();
      auto funcId = [&] {
        std::ostringstream s;
        s << func->getFuncId().toInt();
        return s.str();
      } ();
      StructuredLogEntry entry;
      auto sk = rec->srcKey();
      entry.setStr("trans_kind", show(rec->kind()));
      entry.setStr("callee", sk.func()->fullName()->data());
      entry.setStr("tag", sk.showInst());
      entry.setInt("call_count", callCount);
      entry.setStr("func_id", funcId);
      entry.setStr("uuid", uuidStr);
      if (sk.prologue() || sk.funcEntry()) {
        entry.setInt("num_entry_args", sk.numEntryArgs());
      } else {
        entry.setStr("offset", folly::to<std::string>(sk.offset()));
      }
      StructuredLog::log("hhvm_translation_stats", entry);
    }
  }
}


namespace {
  std::atomic<TransStats*> s_transStats{nullptr};
  struct TransStatsShutdownDeleter {
    ~TransStatsShutdownDeleter() {
      delete s_transStats.load();
    }
  } s_transStatsShutdownDeleter;
} // namespace

TransStats* globalTransStats() {
  return s_transStats.load(std::memory_order_acquire);
}

void processInitTransStats() {
  if (!Cfg::Jit::CollectTranslationStats) return;
  s_transStats.store(new TransStats(), std::memory_order_release);
}

void logGlobalTransStats() {
  auto stats = globalTransStats();
  if (!stats) return;
  stats->logCallCounts();
}

} // namespace HPHP::jit
