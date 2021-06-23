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

#include "hphp/runtime/vm/jit/outlined-sequence-selector.h"

#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/asm-info.h"
#include "hphp/runtime/vm/jit/cfg.h"

#include "hphp/runtime/base/perf-warning.h"
#include "hphp/runtime/base/init-fini-node.h"

#include "hphp/util/trace.h"

#include <folly/hash/Hash.h>

// Outlining first involves identifying what to be outlined.  Ideal candidates
// are repeated code sequences that can be put in common by moving the code to
// a subroutine.  A simple approach to identify such sequences would be a data
// structure that map mapping possible outlined sequences to their counts
// (size, prof count, etc).  In practice this would be massive.  Instead we
// record this information based on a 64bit hash of the sequence.  Assuming our
// hashing is good we should have minimal collisions.  If we have a collision,
// this might mean we outline something unprofitable, but it won't lead to
// correctness issues.
//
// NOTE: The overall data structure size is still large!  (but manageable)
//
// The side effect of only storing hashes, is that we can't generate the
// outlined routine from scratch.  We need to find an occurrence that matches
// the hash in order to generate the outlined routine.  We could save the IR of
// occurrences whose hash reaches a certain profitability, or we could use the
// hashes to generate outlined routines on the fly in a subsequent retranslate
// all.
//
// At the end of the build optimized hashes phase, we are left with a set of
// hashes we would like to optimize.  Subsequent retranslate all runs consume
// this info, and outline the code associated to it.

namespace HPHP { namespace jit {

namespace {

TRACE_SET_MOD(hhir_outline);

using Hash = uint64_t;

// We store a Record for every possible outlined sequence.  It is used to
// decide if outlining a sequence is profitable or not during the build
// optimized hashes phase.
struct Record {
  size_t totalMainBytes;
  uint64_t totalProfCount;
  uint32_t occurrences;
  uint32_t instructions;

  // Records are stored in a map keyed on hashes.  These two hashes allow us to
  // find Records for subsequences of this sequence.  The build optimized
  // hashes phase uses this process to adjust occurrence counts of
  // subsequences as it marks hashes to be outlined.
  Optional<Hash> withoutSuffixHash;
  Optional<Hash> withoutPrefixHash;

  size_t averageMainBytes() const {
    if (!occurrences) return 0;
    return totalMainBytes / occurrences;
  }

  std::string toString() const {
    return folly::sformat("Main bytes: {:,} ({:,} average) IR instrs: {:,} "
                          "Occurrences: {:,} Prof Count: {:,})",
                          totalMainBytes,
                          averageMainBytes(),
                          instructions,
                          occurrences,
                          totalProfCount);
  }

  void reduceCounts(const Record& o) {
    size_t byteReduction = averageMainBytes() * o.occurrences;
    if (totalMainBytes > byteReduction) {
      totalMainBytes -= byteReduction;
    } else {
      totalMainBytes = 0;
    }
    if (occurrences > o.occurrences) {
      occurrences -= o.occurrences;
    } else {
      occurrences = 0;
      totalMainBytes = 0;
    }
    if (totalProfCount > o.totalProfCount) {
      totalProfCount -= o.totalProfCount;
    } else {
      totalProfCount = 0;
    }
  }

  void mergeIn(Hash h, const Record& o) {
    totalMainBytes += o.totalMainBytes;
    totalProfCount += o.totalProfCount;
    occurrences += o.occurrences;
    if (withoutSuffixHash != o.withoutSuffixHash ||
        withoutPrefixHash != o.withoutPrefixHash ||
        instructions != o.instructions) {
      // There has been a hash collision.  The optimization may perform
      // slightly worse when this is the case.  This is only a potential
      // performance issue, not a correctness issue.  Note: we could have
      // undetected collisions.  This is just one of the easily detectable
      // cases.
      DEBUG_ONLY auto const optHashToStr = [] (const Optional<Hash>& h) {
        if (!h.has_value()) return std::string("none");
        return folly::sformat("{:016X}", *h);
      };
      FTRACE(
        1,
        "Hash collision! {:016X} instructions ({}, {}) without suffix ({}, {})"
        " without prefix ({}, {})\n",
        h,
        instructions, o.instructions,
        optHashToStr(withoutSuffixHash), optHashToStr(o.withoutSuffixHash),
        optHashToStr(withoutPrefixHash), optHashToStr(o.withoutPrefixHash)
      );
    }
  }
};

using DataPtr = std::shared_ptr<std::unordered_map<Hash, Record>>;

std::unordered_set<Hash> s_optimizedHashes;
std::mutex s_tlDataStoreListLock;
std::vector<DataPtr> s_tlDataStoreList;
thread_local DataPtr tl_dataStorePtr;
InitFiniNode s_registerTlDataStore([] () {
  std::unique_lock<std::mutex> lock(s_tlDataStoreListLock);
  s_tlDataStoreList.emplace_back(
    std::make_shared<std::unordered_map<Hash, Record>>()
  );
  tl_dataStorePtr = s_tlDataStoreList.back();
}, InitFiniNode::When::ThreadInit);

// The MetaBuilder is used to compose hashes and metadata information
// instruction by instruction.  The largest role the meta builder plays in hash
// creation is to renumber SSATmps based on appearance order.  During IR
// processing multiple MetaBuilders will be in use at the same time.  One
// starting at each instruction in the block.
struct MetaBuilder {
  Optional<Hash> hash{};
  Optional<Hash> chainedFromHash{};
  size_t mainBytes{0};
  uint64_t profCount;
  uint32_t numInstructions{0};

  // SSATmp renumbering.
  std::unordered_map<uint32_t, uint32_t> renumbering{};

  explicit MetaBuilder(uint64_t profCount) : profCount(profCount) {}

  void addInstruction(const IRInstruction* instr, size_t bytes) {
    mainBytes += bytes;
    numInstructions++;

    // Build the hash.  It is important the hash used be stable across HHVM
    // restarts, as they will be serialized into the jumpstart package.
    chainedFromHash = hash;
    if (!hash.has_value()) hash = 0;

    Hash srcsDsts{0};
    auto const srcDstBuilder = [&] (auto const& tmps) {
      for (auto const tmp : tmps) {
        auto const [it, inserted] =
          renumbering.try_emplace(tmp->id(), renumbering.size());
        srcsDsts = folly::hash::hash_combine(
          srcsDsts,
          std::hash<decltype(it->second)>()(it->second),
          tmp->type().stableHash()
        );
      }
    };
    srcDstBuilder(instr->srcs());
    srcDstBuilder(instr->dsts());
    hash = folly::hash::hash_combine(
      *hash,
      std::hash<decltype(instr->op())>()(instr->op()),
      std::hash<decltype(instr->numSrcs())>()(instr->numSrcs()),
      srcsDsts
    );
    if (instr->hasTypeParam()) {
      hash = folly::hash::hash_combine(
        *hash,
        instr->typeParam().stableHash()
      );
    }
    if (instr->hasExtra()) {
      hash = folly::hash::hash_combine(
        *hash,
        stableHashExtra(instr->op(), instr->rawExtra())
      );
    }
  }
};

void recordRecord(Hash hash, const Record& r) {
  auto const [it, inserted] = tl_dataStorePtr->insert({hash, r});
  if (!inserted) it->second.mergeIn(hash, r);
}

bool disallowedOp(const IRInstruction& instr) {
  // isBlockEnd captures all control flow instructions.
  return instr.op() == DefLabel || instr.op() == BeginCatch ||
    instr.op() == DefFP || instr.isBlockEnd();
}


void recordBlock(const Block* block, AsmInfo* ai) {
  std::vector<MetaBuilder>  metaBuilders;
  metaBuilders.reserve(block->instrs().size());

  auto const& mainRanges = ai->instRangesForArea(AreaIndex::Main);
  for (auto const& instr : block->instrs()) {
    if (disallowedOp(instr)) {
      metaBuilders.clear();
      continue;
    }
    auto const& instrRanges = mainRanges[instr];
    size_t mainBytes = 0;
    for (auto it = instrRanges.first; it != instrRanges.second; ++it) {
      mainBytes += it->second.size();
    }
    // Start a builder at each instruction.
    metaBuilders.emplace_back(MetaBuilder{block->profCount()});
    for (auto& mb : metaBuilders) {
      mb.addInstruction(&instr, mainBytes);
    }
    for (size_t i = 0; i < metaBuilders.size(); i++) {
      auto const& mb = metaBuilders[i];
      assertx(mb.hash.has_value());
      Record r {
        mb.mainBytes,
        mb.profCount,
        1,
        mb.numInstructions,
        mb.chainedFromHash,
        i + 1 < metaBuilders.size() ? metaBuilders[i + 1].hash : std::nullopt
      };
      recordRecord(*mb.hash, r);
    }
  }
}

} // anonymous namespace

void buildOptimizedHashes() {
  // Reduce the data into `data` for sorting and processing.
  s_optimizedHashes.clear();
  std::unordered_map<Hash, Record> data;
  {
    std::unique_lock<std::mutex> lock(s_tlDataStoreListLock);
    for (auto const& dPtr : s_tlDataStoreList) {
      for (auto const& e : *dPtr) {
        auto const [it, inserted] = data.insert(e);
        if (!inserted) it->second.mergeIn(e.first, e.second);
      }
      dPtr->clear();
    }
  }

  // Define an ordering in which to perform optimization of eligible hashes.
  // Favor optimizing longer instruction sequences first to ensure that their
  // subsequences are properly adjusted.  This doesn't necessarily work as well
  // as a favor sequences with higher total main size, but that would lead to
  // potentially incorrect adjustment during reduceCounts.
  auto orderCmp = [&](const Hash &lHash, const Hash &rHash) {
    auto const& l = data[lHash];
    auto const& r = data[rHash];
    if (l.instructions != r.instructions) {
      // If instructions is not equal then hashes are not equal, so we can
      // return any well defined ordering based on instructions without
      // disturbing std::sets assumptions about comparison.
      return l.instructions > r.instructions;
    }
    // Arbitrary well defined ordering to keep std::set happy.
    return lHash > rHash;
  };
  std::set<Hash, decltype(orderCmp)> order(orderCmp);
  size_t optimizedBytes{0};

  uint32_t k_OccurrenceThreshold = 10;
  size_t k_MainBytesThreshold = 64;
  // This is the size of a call instruction with a 32bit immediate.  We
  // optimistically  assume the register allocator can avoid all parameter
  // shuffles.
  size_t k_EstimatedCallOverheadBytes = 5;
  auto const insert = [&] (Hash h) {
    // Filter hashes that don't meet optimization criteria.
    auto const& r = data[h];
    if (r.occurrences < k_OccurrenceThreshold) return;
    if (r.averageMainBytes() < k_MainBytesThreshold) return;
    order.insert(h);
  };
  auto const reduceCounts = [&] (Hash h, const Record& removed) {
    // It initially seems like we could just store hashes, but a hash may
    // appear as a subsequence multiple times.  Instead we store pair of ints
    // representing the number of prefixes/suffixes removed.
    std::unordered_set<std::pair<uint32_t, uint32_t>> visited;
    auto const impl = [&] (Hash h, const Record& removed,
                           uint32_t suffixesRemoved,
                           uint32_t prefixesRemoved,
                           auto const& impl_ref) {
      std::pair<uint32_t, uint32_t> key(suffixesRemoved, prefixesRemoved);
      if (visited.count(key)) return;
      visited.insert(key);
      order.erase(h);
      auto& r = data[h];
      r.reduceCounts(removed);
      insert(h);
      if (r.withoutSuffixHash.has_value()) {
        impl_ref(*r.withoutSuffixHash, removed, suffixesRemoved + 1,
                 prefixesRemoved, impl_ref);
      }
      if (r.withoutPrefixHash.has_value()) {
        impl_ref(*r.withoutPrefixHash, removed, suffixesRemoved,
                 prefixesRemoved + 1, impl_ref);
      }
    };
    return impl(h, removed, 0, 0, impl);
  };
  auto const optimize = [&] (Hash h) {
    s_optimizedHashes.insert(h);
    auto const r = data[h];
    reduceCounts(h, r);

    FTRACE(4, "Marking hash {:016X} for optimization ({})\n", h, r);
    optimizedBytes += r.totalMainBytes;
    optimizedBytes -= r.averageMainBytes();
    optimizedBytes -= r.occurrences * k_EstimatedCallOverheadBytes;
  };

  for (auto const& e : data) {
    insert(e.first);
  }

  while (!order.empty()) {
    optimize(*order.begin());
  }

  FTRACE(1, "Marked {} hashes for optimization ({:,} main bytes saved)\n",
         s_optimizedHashes.size(), optimizedBytes);
}

void recordIR(const IRUnit& unit, AsmInfo* ai) {
  auto const& ctx = unit.context();
  auto const kind = ctx.kind;
  if (kind != TransKind::Optimize) return;

  // For now the order we walk the unit doesn't particularly matter.  Post
  // order is convenient so:
  postorderWalk(unit, [&] (const Block* b) {
    recordBlock(b, ai);
  });
}

}} // namespace HPHP, jit
