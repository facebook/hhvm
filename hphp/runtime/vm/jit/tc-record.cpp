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

#include "hphp/runtime/vm/jit/tc-record.h"

#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/tc-region.h"

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/trans-rec.h"

#include "hphp/util/data-block.h"
#include "hphp/util/service-data.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include <folly/gen/Base.h>
#include <folly/json.h>

TRACE_SET_MOD(mcg);

namespace HPHP::jit::tc {

void recordGdbTranslation(SrcKey sk, const CodeBlock& cb,
                          const TCA start, const TCA end) {
  assertx(cb.contains(start, end));
  if (start != end) {
    assertOwnsCodeLock();
    if (!RuntimeOption::EvalJitNoGdb) {
      Debug::DebugInfo::Get()->recordTracelet(
        Debug::TCRange(start, end, &cb == &code().cold()),
        sk
      );
    }
    if (RuntimeOption::EvalPerfPidMap) {
      Debug::DebugInfo::Get()->recordPerfMap(
        Debug::TCRange(start, end, &cb == &code().cold()),
        sk
      );
    }
  }
}

void recordBCInstr(uint32_t op, const TCA addr, const TCA end, bool cold) {
  if (addr != end) {
    Debug::DebugInfo::Get()->recordBCInstr(Debug::TCRange(addr, end, cold), op);
  }
}

////////////////////////////////////////////////////////////////////////////////

std::map<std::string, ServiceData::ExportedTimeSeries*>
buildCodeSizeCounters() {
  std::map<std::string, ServiceData::ExportedTimeSeries*> counters;
  auto codeCounterInit =
    [&] (const char* name,
         const std::vector<ServiceData::StatsType>& stats) {
      auto counterName = folly::sformat("jit.code.{}.used", name);
      counters[name] = ServiceData::createTimeSeries(
          counterName, stats,
          {std::chrono::seconds(RuntimeOption::EvalJitWarmupRateSeconds),
           std::chrono::seconds(0)}
      );
    };
  auto codeCounterSumAndRateInit = [&] (const char* name) {
    codeCounterInit(name, {ServiceData::StatsType::RATE,
                           ServiceData::StatsType::SUM});
  };
  auto codeCounterSumInit = [&] (const char* name) {
    codeCounterInit(name, {ServiceData::StatsType::SUM});
  };
  CodeCache::forEachName(codeCounterSumAndRateInit);
  codeCounterSumInit("prof.main");
  codeCounterSumInit("prof.cold");
  codeCounterSumInit("prof.frozen");
  codeCounterSumInit("opt.main");
  codeCounterSumInit("opt.cold");
  codeCounterSumInit("opt.frozen");
  codeCounterSumInit("live.main");
  codeCounterSumInit("live.cold");
  codeCounterSumInit("live.frozen");
  return counters;
}

static std::map<std::string, ServiceData::ExportedTimeSeries*> s_counters;

static std::atomic<uint64_t> s_trans_counters[NumTransKinds][kNumAreas];

static InitFiniNode initCodeSizeCounters([] {
  s_counters = buildCodeSizeCounters();
}, InitFiniNode::When::PostRuntimeOptions);

static std::atomic<bool> s_warmedUp{false};

static ServiceData::CounterCallback s_warmedUpCounter(
  [](std::map<std::string, int64_t>& counters) {
    counters["jit.warmed-up"] = warmupStatusString().empty() ? 1 : 0;
  }
);

/*
 * Return the numeric index to use for the given kind for the purpose of
 * recording the translation sizes in s_trans_counters.  This is normally the
 * order of the given kind in the TransKind enum class, except that prologue
 * kinds get mapped to their corresponding non-prologue kinds.
 */
static size_t kindIndex(TransKind kind) {
  switch (kind) {
    case TransKind::ProfPrologue:
      kind = TransKind::Profile;
      break;
    case TransKind::LivePrologue:
      kind = TransKind::Live;
      break;
    case TransKind::OptPrologue:
      kind = TransKind::Optimize;
      break;
    default:
      break;
  }
  return static_cast<size_t>(kind);
}

void recordTranslationSizes(const TransRec& tr) {
  const char* kindName = nullptr;
  switch (tr.kind) {
    case TransKind::Profile:
    case TransKind::ProfPrologue:
      kindName = "prof";
      break;
    case TransKind::Optimize:
    case TransKind::OptPrologue:
      kindName = "opt";
      break;
    case TransKind::Live:
    case TransKind::LivePrologue:
      kindName = "live";
      break;
    // We don't record the other, less common translation kinds.
    default:
      return;
  }
  auto mainCounter   = s_counters.at(folly::sformat("{}.main", kindName));
  auto coldCounter   = s_counters.at(folly::sformat("{}.cold", kindName));
  auto frozenCounter = s_counters.at(folly::sformat("{}.frozen", kindName));
  mainCounter->addValue(tr.aLen);
  coldCounter->addValue(tr.acoldLen);
  frozenCounter->addValue(tr.afrozenLen);

  size_t kindIdx = kindIndex(tr.kind);
  auto& trans_counter = s_trans_counters[kindIdx];
  auto constexpr iMain   = static_cast<size_t>(AreaIndex::Main);
  auto constexpr iCold   = static_cast<size_t>(AreaIndex::Cold);
  auto constexpr iFrozen = static_cast<size_t>(AreaIndex::Frozen);
  trans_counter[iMain].fetch_add(tr.aLen, std::memory_order_relaxed);
  trans_counter[iCold].fetch_add(tr.acoldLen, std::memory_order_relaxed);
  trans_counter[iFrozen].fetch_add(tr.afrozenLen, std::memory_order_relaxed);
}

void updateCodeSizeCounters() {
  assertOwnsCodeLock();

  code().forEachBlock([&] (const char* name, const CodeBlock& a) {
    auto codeUsed = s_counters.at(name);
    // Add delta
    codeUsed->addValue(a.used() - codeUsed->getSum());
  });

  // Manually add code.data.
  auto codeUsed = s_counters.at("data");
  codeUsed->addValue(code().data().used() - codeUsed->getSum());
}

size_t getLiveMainUsage() {
  constexpr auto liveIdx = static_cast<size_t>(TransKind::Live);
  constexpr auto mainIdx = static_cast<size_t>(AreaIndex::Main);
  return s_trans_counters[liveIdx][mainIdx].load(std::memory_order_relaxed);
}

size_t getProfMainUsage() {
  constexpr auto profIdx = static_cast<size_t>(TransKind::Profile);
  constexpr auto mainIdx = static_cast<size_t>(AreaIndex::Main);
  return s_trans_counters[profIdx][mainIdx].load(std::memory_order_relaxed);
}

size_t getOptMainUsage() {
  constexpr auto optIdx = static_cast<size_t>(TransKind::Optimize);
  constexpr auto mainIdx = static_cast<size_t>(AreaIndex::Main);
  return s_trans_counters[optIdx][mainIdx].load(std::memory_order_relaxed);
}

/*
 * Update JIT maturity with the current amount of emitted code and state of the
 * JIT.
 */
void reportJitMaturity() {
  auto static jitMaturityCounter = ServiceData::createCounter("jit.maturity");
  if (!jitMaturityCounter) return;
  auto const before = jitMaturityCounter->getValue();
  if (before == 100) return;

  // Limit jit maturity to 70 before retranslateAll finishes (if enabled). If
  // the JIT running in jumpstart seeer mode, don't consider retranslateAll to
  // ever finish.
  constexpr uint64_t kMaxMaturityBeforeRTA = 70;
  auto const beforeRetranslateAll =
    mcgen::retranslateAllPending() || isJitSerializing();
  // If retranslateAll is enabled, wait until it finishes before counting in
  // optimized translations.
  const size_t hotSize = beforeRetranslateAll ? 0 : getOptMainUsage();
  const size_t liveSize = getLiveMainUsage();
  // When we jit from serialized profile data, the profile code won't be
  // present. In order to make jit maturity somewhat comparable between the two
  // cases, we pretend to have some profiling code.
  //
  // If we've restarted to reload profiling data before RTA, the profile code
  // won't be present. Since hotSize and mainSize will also be zero in this
  // case, our maturity will sit at 0. To avoid this, we've recorded the size of
  // the main profile code before we restarted with the profiling data. Use that
  // to simulate the size of the profile code.
  auto const profSize = std::max(
    getProfMainUsage() + (isJitSerializing() ? ProfData::prevProfSize() : 0),
    hotSize
  );
  auto const codeSize =
    std::max(hotSize + liveSize,
             static_cast<size_t>(profSize * RO::EvalJitMaturityProfWeight));
  auto const fullSize = RO::EvalJitMatureSize;

  int64_t maturity = before;
  if (beforeRetranslateAll) {
    maturity = std::min(kMaxMaturityBeforeRTA, codeSize * 100 / fullSize);
  } else if (liveSize >= RO::EvalJitMaxLiveMainUsage ||
             code().main().used() >= CodeCache::AMaxUsage ||
             code().cold().used() >= CodeCache::AColdMaxUsage ||
             code().frozen().used() >= CodeCache::AFrozenMaxUsage) {
    maturity = 100;
  } else if (codeSize >= fullSize) {
    maturity = 99;
  } else {
    maturity = std::pow(codeSize / static_cast<double>(fullSize),
                        RuntimeOption::EvalJitMaturityExponent) * 99;
  }

  // If EvalJitMatureAfterWarmup is set, we consider the JIT to be mature once
  // warmupStatusString() is empty, which indicates that the JIT is warmed up
  // based on the rate in which JITed code is being produced.
  if (RuntimeOption::EvalJitMatureAfterWarmup && warmupStatusString().empty()) {
    maturity = 100;
  }

  if (maturity > before) {
    jitMaturityCounter->setValue(maturity);
  }
}

static void logFrame(const Vunit& unit, const size_t frame) {
  std::vector<const Func*> funcs;
  for (auto f = frame; f != Vframe::Top; f = unit.frames[f].parent) {
    funcs.emplace_back(unit.frames[f].func);
  }
  auto gens = folly::gen::from(funcs);

  auto esc = [&] (const char* s) {
    std::string ret;
    folly::json::escapeString(s, ret, folly::json::serialization_opts());
    return ret;
  };

#define MAP(e) \
  gens | folly::gen::mapped([&] (const Func* f) -> std::string {            \
    return esc(e);                                                          \
  }) | folly::gen::as<std::vector>()

  auto fullnames = MAP(f->fullName()->data());
  auto names = MAP(f->name()->data());
  auto impl_classes = MAP(f->implCls() ? f->implCls()->name()->data() : "");
  auto base_classes = MAP(f->baseCls() ? f->baseCls()->name()->data() : "");
  auto ctx_classes = MAP(f->cls() ? f->cls()->name()->data() : "");
  auto files = MAP(f->unit()->origFilepath()->data());

#undef MAP

#define CAST(t, v) std::t<folly::StringPiece>(v.begin(), v.end())

  StructuredLogEntry ent;
  ent.setStr("inlined", funcs.size() != 1 ? "true" : "false");

  auto const func = funcs.front();

  if (auto cls = func->cls()) {
    std::vector<std::string> interfaces;
    std::vector<std::string> traits;
    std::vector<std::string> parents;

    for (auto iface : cls->allInterfaces().range()) {
      interfaces.emplace_back(esc(iface->name()->data()));
    }
    for (auto trait : cls->preClass()->usedTraits()) {
      traits.emplace_back(esc(trait->data()));
    }
    for (auto c = cls; c; c = c->parent()) {
      parents.emplace_back(esc(c->name()->data()));
    }

    ent.setSet("traits", CAST(set, traits));
    ent.setSet("interfaces", CAST(set, interfaces));
    ent.setVec("parent_classes", CAST(vector, parents));
  }

#define SET(nm, vec)                            \
  ent.setStr(#nm, vec.front());                 \
  ent.setVec("inlined_" #nm, CAST(vector, vec))

  SET(full_function_name, fullnames);
  SET(function_name, names);
  SET(impl_class, impl_classes);
  SET(base_class, base_classes);
  SET(class, ctx_classes);
  SET(file, files);

#undef SET
#undef CAST

  size_t inclusive = 0;
  size_t exclusive = 0;
  for (uint8_t idx = 0; idx < kNumAreas; ++idx) {
    auto const aidx = static_cast<AreaIndex>(idx);
    auto const inm = folly::sformat("{}_inclusive_bytes", areaAsString(aidx));
    auto const enm = folly::sformat("{}_exclusive_bytes", areaAsString(aidx));
    auto const ibytes = unit.frames[frame].sections[idx].inclusive;
    auto const ebytes = unit.frames[frame].sections[idx].exclusive;
    ent.setInt(inm, ibytes);
    ent.setInt(enm, ebytes);
    inclusive += ibytes;
    exclusive += ebytes;
  }

  ent.setInt("total_inclusive_bytes", inclusive);
  ent.setInt("total_exclusive_bytes", exclusive);

  ent.setInt("inclusive_cost", unit.frames[frame].inclusive_cost);
  ent.setInt("exclusive_cost", unit.frames[frame].exclusive_cost);
  ent.setInt("num_inner_frames", unit.frames[frame].num_inner_frames);
  ent.setInt("entry_weight", unit.frames[frame].entry_weight);

  auto const sk = unit.context->initSrcKey;
  ent.setStr("version", "6");
  ent.setStr("trans_kind", show(unit.context->kind));
  ent.setStr("prologue", sk.prologue() ? "true" : "false");
  ent.setStr("func_entry", sk.funcEntry() ? "true" : "false");
  ent.setStr("has_this", sk.hasThis() ? "true" : "false");
  ent.setStr("resumed", sk.resumeMode() != ResumeMode::None
                        ? "true" : "false");

  logFunc(func, ent);

  if (!RuntimeOption::EvalJitLogAllInlineRegions.empty()) {
    ent.setStr("run_key", RuntimeOption::EvalJitLogAllInlineRegions);
  }

  StructuredLog::log("hhvm_tc_func_sizes", ent);
}

void logFrames(const Vunit& unit) {
  if (!unit.context || unit.frames.empty() || !unit.frames.front().func) return;
  for (size_t frame = 0; frame < unit.frames.size(); ++frame) {
    logFrame(unit, frame);
  }
}

void logTranslation(const Translator* trans, const TransRange& range) {
  auto nanos = HPHP::Timer::GetThreadCPUTimeNanos() - trans->unit->startNanos();
  auto& cols = *trans->unit->logEntry();
  auto kind = show(trans->kind);
  cols.setStr("trans_kind", !debug ? kind : kind + "_debug");
  if (trans->sk.valid()) {
    auto const func = trans->sk.func();
    cols.setStr("func", func->fullName()->data());
    switch (RuntimeOption::EvalJitSerdesMode) {
    case JitSerdesMode::Off:
    case JitSerdesMode::Serialize:
    case JitSerdesMode::SerializeAndExit:
      break;
    case JitSerdesMode::Deserialize:
    case JitSerdesMode::DeserializeOrFail:
    case JitSerdesMode::DeserializeOrGenerate:
    case JitSerdesMode::DeserializeAndDelete:
    case JitSerdesMode::DeserializeAndExit:
      cols.setInt("func_id", func->getFuncId().toInt());
      break;
    }
  }
  if (trans->kind == TransKind::Optimize) {
    auto const regionTrans = dynamic_cast<const RegionTranslator*>(trans);
    assertx(regionTrans);
    cols.setInt("opt_index", regionTrans->optIndex);
  }
  cols.setInt("jit_sample_rate", RuntimeOption::EvalJitSampleRate);
  // timing info
  cols.setInt("jit_micros", nanos / 1000);
  // hhir stats
  cols.setInt("max_tmps", trans->unit->numTmps());
  cols.setInt("max_blocks", trans->unit->numBlocks());
  cols.setInt("max_insts", trans->unit->numInsts());
  auto hhir_blocks = rpoSortCfg(*trans->unit);
  cols.setInt("num_blocks", hhir_blocks.size());
  size_t num_insts = 0;
  for (auto b : hhir_blocks) num_insts += b->instrs().size();
  cols.setInt("num_insts", num_insts);
  // vasm stats
  if (trans->vunit) {
    cols.setInt("max_vreg", trans->vunit->next_vr);
    cols.setInt("max_vblocks", trans->vunit->blocks.size());
    cols.setInt("max_vcalls", trans->vunit->vcallArgs.size());
    size_t max_vinstr = 0;
    for (auto& blk : trans->vunit->blocks) max_vinstr += blk.code.size();
    cols.setInt("max_vinstr", max_vinstr);
    cols.setInt("num_vconst", trans->vunit->constToReg.size());
    auto vblocks = sortBlocks(*trans->vunit);
    size_t num_vinstr[kNumAreas] = {0, 0, 0};
    size_t num_vblocks[kNumAreas] = {0, 0, 0};
    for (auto b : vblocks) {
      const auto& block = trans->vunit->blocks[b];
      num_vinstr[(int)block.area_idx] += block.code.size();
      num_vblocks[(int)block.area_idx]++;
    }
    cols.setInt("num_vinstr_main", num_vinstr[(int)AreaIndex::Main]);
    cols.setInt("num_vinstr_cold", num_vinstr[(int)AreaIndex::Cold]);
    cols.setInt("num_vinstr_frozen", num_vinstr[(int)AreaIndex::Frozen]);
    cols.setInt("num_vblocks_main", num_vblocks[(int)AreaIndex::Main]);
    cols.setInt("num_vblocks_cold", num_vblocks[(int)AreaIndex::Cold]);
    cols.setInt("num_vblocks_frozen", num_vblocks[(int)AreaIndex::Frozen]);

    if (RuntimeOption::EvalJitLogAllInlineRegions.empty()) {
      logFrames(*trans->vunit);
    }
  }
  // x64 stats
  cols.setInt("main_size", range.main.size());
  cols.setInt("cold_size", range.cold.size());
  cols.setInt("frozen_size", range.frozen.size());

  // finish & log
  StructuredLog::log("hhvm_jit", cols);
}

std::string warmupStatusString() {
  // This function is like a request-agnostic version of
  // server_warmup_status().
  // Three conditions necessary for the jit to qualify as "warmed-up":
  std::string status_str;

  if (!s_warmedUp.load(std::memory_order_relaxed)) {
    if (jit::mcgen::retranslateAllPending()) {
      status_str = "Waiting on retranslateAll().\n";
    } else {
      auto checkCodeSize = [&](std::string name, uint32_t maxSize) {
        assertx(!s_counters.empty());
        auto series = s_counters.at(name);
        if (!series) {
          status_str = "initializing";
          return;
        }
        auto const codeSize = series->getSum();
        if (codeSize < maxSize / RuntimeOption::EvalJitWarmupMinFillFactor) {
          folly::format(&status_str,
                        "Code.{} is still to small to be considered warm. "
                        "({} of max {})\n",
                        name, codeSize, maxSize);
          return;
        }
        auto const codeSizeRate = series->getRateByDuration(
          std::chrono::seconds(RuntimeOption::EvalJitWarmupRateSeconds));
        if (codeSizeRate > RuntimeOption::EvalJitWarmupMaxCodeGenRate) {
          folly::format(&status_str,
                        "Code.{} is still increasing at a rate of {}\n",
                        name, codeSizeRate);
        }
      };
      checkCodeSize("main", CodeCache::ASize);
    }
    if (status_str.empty()) {
      if (RuntimeOption::EvalJitSerdesMode == JitSerdesMode::SerializeAndExit) {
        status_str = "JIT running in SerializeAndExit mode";
      } else {
        s_warmedUp.store(true, std::memory_order_relaxed);
      }
    }
  }
  // Empty string means "warmed up".
  return status_str;
}

}
