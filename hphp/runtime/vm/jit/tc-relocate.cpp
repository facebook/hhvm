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

#include "hphp/runtime/vm/jit/tc-relocate.h"

#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/relocation.h"

#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/stub-alloc.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"

#include "hphp/tools/hfsort/jitsort.h"

#include "hphp/util/arch.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/logger.h"
#include "hphp/util/trace.h"

#include "hphp/ppc64-asm/asm-ppc64.h"

#include <algorithm>
#include <cstdio>
#include <vector>

namespace HPHP { namespace jit { namespace tc {

//////////////////////////////////////////////////////////////////////

struct TransRelocInfo {
  TransRelocInfo() {}

  TransRelocInfo(TransRelocInfo&&) = default;
  TransRelocInfo& operator=(TransRelocInfo&&) = default;
  TransRelocInfo(const TransRelocInfo&) = delete;
  TransRelocInfo& operator=(const TransRelocInfo&) = delete;
  SrcKey sk;
  int argNum;
  TCA start;
  TCA end;
  TCA coldStart;
  TCA coldEnd;
  GrowableVector<IncomingBranch> incomingBranches;
  CGMeta fixups;
};

//////////////////////////////////////////////////////////////////////

namespace {

TRACE_SET_MOD(mcg);

bool okToRelocate = true;

template<class T> void swap_trick(T& container) { T().swap(container); }

/*
 * CodeSmasher is used to overwrite all the relocated code with ud2
 * and int3 to ensure that we never execute it after the code has
 * been relocated. Its run as a treadmill job, because old requests
 * might continue to execute the old code, even after we finish
 * the relocation step.
 */
struct CodeSmasher {
  std::vector<std::pair<TCA,TCA>> entries;
  void operator()() {
    auto codeLock = lockCode();

    for (auto& e : entries) {
      CodeBlock cb;
      cb.init(e.first, e.second - e.first, "relocated");

      CGMeta fixups;
      SCOPE_EXIT { assert(fixups.empty()); };

      DataBlock db;
      Vauto vasm { cb, cb, db, fixups };
      vasm.unit().padding = true;
    }
    okToRelocate = true;
  }
};

struct PostProcessParam {
  RelocationInfo& rel;
  std::set<TCA>&  deadStubs;
  FILE*           relocMap;
};

void postProcess(TransRelocInfo&& tri, void* paramPtr) {
  auto& param = *static_cast<PostProcessParam*>(paramPtr);
  auto& rel = param.rel;
  auto& deadStubs = param.deadStubs;
  auto relocMap = param.relocMap;

  if (!rel.adjustedAddressAfter(tri.start)) {
    adjustForRelocation(rel, tri.start, tri.end);
    auto coldStart = tri.coldStart;
    if (&code().blockFor(coldStart) == &code().frozen()) {
      /*
       * This is a bit silly. If we were generating code into frozen, and we
       * also put stubs in frozen, and those stubs are now dead the code in
       * them isn't valid (we smashed the first few bytes with a pointer and a
       * size; see FreeStubList::StubNode).  So skip over any of those. Its ok
       * to process the live ones more than once.
       */
      auto it = deadStubs.lower_bound(tri.coldStart);
      while (it != deadStubs.end() && *it < tri.coldEnd) {
        adjustForRelocation(rel, coldStart, *it);
        coldStart = *it + svcreq::stub_size();
        ++it;
      }
    }
    adjustForRelocation(rel, coldStart, tri.coldEnd);
  }
  auto adjustAfter = [&rel](TCA& addr) {
    if (auto adj = rel.adjustedAddressAfter(addr)) addr = adj;
  };
  auto adjustBefore = [&rel](TCA& addr) {
    if (auto adj = rel.adjustedAddressBefore(addr)) addr = adj;
  };

  adjustAfter(tri.start);
  adjustBefore(tri.end);
  adjustAfter(tri.coldStart);
  adjustBefore(tri.coldEnd);

  for (auto& ib : tri.incomingBranches) {
    if (auto adjusted = rel.adjustedAddressAfter(ib.toSmash())) {
      ib.adjust(adjusted);
    }
  }
  adjustMetaDataForRelocation(rel, nullptr, tri.fixups);

  auto data = perfRelocMapInfo(
    tri.start, tri.end,
    tri.coldStart, tri.coldEnd,
    tri.sk, tri.argNum,
    tri.incomingBranches,
    tri.fixups
  );

  fprintf(relocMap, "%" PRIxPTR " %" PRIxPTR " %s\n",
          intptr_t(tri.start), intptr_t(tri.end),
          data.c_str());
}

void readRelocsIntoVector(TransRelocInfo&& tri, void* data) {
  if (code().prof().contains(tri.start)) return;
  auto v = static_cast<std::vector<TransRelocInfo>*>(data);
  v->emplace_back(std::move(tri));
}

bool readLine(std::string& out, FILE* file) {
  out.resize(0);
  while (true) {
    int c = fgetc(file);
    if (c == EOF) return out.size();
    if (c == '\n') return true;
    out.push_back(c);
  }
}

struct TransRelocInfoHelper {
  SrcKey::AtomicInt skInt;
  int argNum;
  std::pair<uint32_t,uint32_t> coldRange;
  std::vector<IncomingBranch::Opaque> incomingBranches;
  std::vector<uint32_t> addressImmediates;
  std::vector<uint64_t> codePointers;
  std::vector<std::pair<uint32_t,std::pair<Alignment,AlignContext>>> alignments;

  template<class SerDe> void serde(SerDe& sd) {
    sd
      (skInt)
      (argNum)
      (coldRange)
      (incomingBranches)
      (addressImmediates)
      (codePointers)
      (alignments)
      ;
  }

  TransRelocInfo toTRI(const CodeCache& code) {
    TransRelocInfo tri;
    tri.sk = SrcKey(skInt);
    tri.argNum = argNum;
    tri.coldStart = code.base() + coldRange.first;
    tri.coldEnd = code.base() + coldRange.second;

    for (auto& ib : incomingBranches) {
      tri.incomingBranches.push_back(IncomingBranch(ib));
    }
    for (auto& ai : addressImmediates) {
      tri.fixups.addressImmediates.insert(ai + code.base());
    }
    for (auto& cp : codePointers) {
      tri.fixups.codePointers.emplace((TCA*)cp);
    }
    for (auto v : alignments) {
      tri.fixups.alignments.emplace(v.first + code.base(), v.second);
    }
    return tri;
  }
};

void relocateStubs(TransLoc& loc, TCA frozenStart, TCA frozenEnd,
                   RelocationInfo& rel, CodeCache::View cache,
                   CGMeta& fixups) {
  auto const stubSize = svcreq::stub_size();

  for (auto addr : fixups.reusedStubs) {
    if (!loc.contains(addr)) continue;
    always_assert(frozenStart <= addr);

    CodeBlock dest;
    dest.init(cache.frozen().frontier(), stubSize, "New Stub");
    relocate(rel, dest, addr, addr + stubSize, cache.frozen(), fixups, nullptr,
             AreaIndex::Frozen);
    cache.frozen().skip(stubSize);
    if (addr != frozenStart) {
      rel.recordRange(frozenStart, addr, frozenStart, addr);
    }
    frozenStart = addr + stubSize;
  }
  if (frozenStart != frozenEnd) {
    rel.recordRange(frozenStart, frozenEnd, frozenStart, frozenEnd);
  }

  adjustForRelocation(rel);
  adjustMetaDataForRelocation(rel, nullptr, fixups);
  adjustCodeForRelocation(rel, fixups);
}

////////////////////////////////////////////////////////////////////////////////

void readRelocations(
  FILE* relocFile,
  std::set<TCA>* liveStubs,
  void (*callback)(TransRelocInfo&& tri, void* data),
  void* data) {
  std::string line;
  uint64_t addr;
  uint64_t end;

  while (readLine(line, relocFile)) {
    int n;
    if (sscanf(line.c_str(), "%" SCNx64 " %" SCNx64 "%n",
               &addr, &end, &n) >= 2) {
      auto pos = line.rfind(' ');
      if (pos == n) {
        if (liveStubs && !end) {
          if (!strcmp("NewStub", line.c_str() + pos + 1)) {
            liveStubs->insert((TCA)addr);
          } else if (!strcmp("FreeStub", line.c_str() + pos + 1)) {
            liveStubs->erase((TCA)addr);
          }
        }
        continue;
      }
      assert(pos != std::string::npos && pos > n);
      auto b64 = line.substr(pos + 1);
      auto decoded = base64_decode(b64.c_str(), b64.size(), true);

      BlobDecoder blob(decoded.data(), decoded.size());
      TransRelocInfoHelper trih;
      blob(trih);

      TransRelocInfo tri(trih.toTRI(code()));
      tri.start = reinterpret_cast<TCA>(addr);
      tri.end = reinterpret_cast<TCA>(end);
      findFixups(tri.start, tri.end, tri.fixups);

      callback(std::move(tri), data);
    }
  }
}

void adjustProfiledCallers(RelocationInfo& rel) {
  auto pd = profData();
  if (!pd) return;

  auto updateCallers = [&] (std::vector<TCA>& callers) {
    for (auto& caller : callers) {
      if (auto adjusted = rel.adjustedAddressAfter(caller)) {
        caller = adjusted;
      }
    }
  };

  pd->forEachTransRec([&] (ProfTransRec* rec) {
    if (rec->kind() != TransKind::ProfPrologue) return;
    auto lock = rec->lockCallerList();
    updateCallers(rec->mainCallers());
    updateCallers(rec->guardCallers());
  });
}

void relocate(std::vector<TransRelocInfo>& relocs, CodeBlock& dest,
              CGMeta& fixups) {
  assertOwnsCodeLock();
  assert(!Func::s_treadmill);

  auto newRelocMapName = Debug::DebugInfo::Get()->getRelocMapName() + ".tmp";
  auto newRelocMap = fopen(newRelocMapName.c_str(), "w+");
  if (!newRelocMap) return;

  SCOPE_EXIT {
    if (newRelocMap) fclose(newRelocMap);
  };

  Func::s_treadmill = true;
  SCOPE_EXIT {
    Func::s_treadmill = false;
  };

  auto ignoreEntry = [](const SrcKey& sk) {
    // We can have entries such as UniqueStubs with no SrcKey
    // These are ok to process.
    if (!sk.valid()) return false;
    // But if the func has been removed from the AtomicHashMap,
    // we don't want to process it.
    return !Func::isFuncIdValid(sk.funcID());
  };

  RelocationInfo rel;
  size_t num = 0;
  assert(fixups.alignments.empty());
  for (size_t sz = relocs.size(); num < sz; num++) {
    auto& reloc = relocs[num];
    if (ignoreEntry(reloc.sk)) continue;
    auto start DEBUG_ONLY = dest.frontier();
    try {
      auto& srcBlock = code().blockFor(reloc.start);
      relocate(rel, dest,
               reloc.start, reloc.end, srcBlock, reloc.fixups, nullptr,
               AreaIndex::Main);
    } catch (const DataBlockFull& dbf) {
      break;
    }
    if (Trace::moduleEnabledRelease(Trace::mcg, 1)) {
      Trace::traceRelease(
        folly::sformat("Relocate: 0x{:08x}+0x{:04x} => 0x{:08x}+0x{:04x}\n",
                       (uintptr_t)reloc.start, reloc.end - reloc.start,
                       (uintptr_t)start, dest.frontier() - start));
    }
  }
  swap_trick(fixups.alignments);
  assert(fixups.empty());

  adjustForRelocation(rel);

  for (size_t i = 0; i < num; i++) {
    if (!ignoreEntry(relocs[i].sk)) {
      adjustMetaDataForRelocation(rel, nullptr, relocs[i].fixups);
    }
  }

  for (size_t i = 0; i < num; i++) {
    if (!ignoreEntry(relocs[i].sk)) {
      relocs[i].fixups.process_only(nullptr);
    }
  }

  adjustProfiledCallers(rel);

  // At this point, all the relocated code should be correct, and runable.
  // But eg if it has unlikely paths into cold code that has not been relocated,
  // then the cold code will still point back to the original, not the relocated
  // versions. Similarly reusable stubs will still point to the old code.
  // Since we can now execute the relocated code, its ok to start fixing these
  // things now.

  for (auto& it : srcDB()) {
    it.second->relocate(rel);
  }

  std::unordered_set<Func*> visitedFuncs;
  CodeSmasher s;
  for (size_t i = 0; i < num; i++) {
    auto& reloc = relocs[i];
    if (ignoreEntry(reloc.sk)) continue;
    for (auto& ib : reloc.incomingBranches) {
      ib.relocate(rel);
    }
    if (!reloc.sk.valid()) continue;
    auto f = const_cast<Func*>(reloc.sk.func());

    adjustCodeForRelocation(rel, reloc.fixups);
    reloc.fixups.clear();

    // fixup code references in the corresponding cold block to point
    // to the new code
    adjustForRelocation(rel, reloc.coldStart, reloc.coldEnd);

    if (visitedFuncs.insert(f).second) {
      if (auto adjusted = rel.adjustedAddressAfter(f->getFuncBody())) {
        f->setFuncBody(adjusted);
      }
      auto prologueNum = f->numPrologues();
      while (prologueNum--) {
        auto addr = f->getPrologue(prologueNum);
        if (auto adjusted = rel.adjustedAddressAfter(addr)) {
          f->setPrologue(prologueNum, adjusted);
        }
      }
    }
    if (reloc.end != reloc.start) {
      s.entries.emplace_back(reloc.start, reloc.end);
    }
  }

  auto relocMap = Debug::DebugInfo::Get()->getRelocMap();
  always_assert(relocMap);
  fseek(relocMap, 0, SEEK_SET);

  auto deadStubs = getFreeTCStubs();
  auto param = PostProcessParam { rel, deadStubs, newRelocMap };
  std::set<TCA> liveStubs;
  readRelocations(relocMap, &liveStubs, postProcess, &param);

  // ensure that any reusable stubs are updated for the relocated code
  for (auto stub : liveStubs) {
    FTRACE(1, "Stub: 0x{:08x}\n", (uintptr_t)stub);
    fixups.reusedStubs.emplace_back(stub);
    always_assert(!rel.adjustedAddressAfter(stub));
    fprintf(newRelocMap, "%" PRIxPTR " 0 %s\n", uintptr_t(stub), "NewStub");
  }
  adjustCodeForRelocation(rel, fixups);

  unlink(Debug::DebugInfo::Get()->getRelocMapName().c_str());
  rename(newRelocMapName.c_str(),
         Debug::DebugInfo::Get()->getRelocMapName().c_str());
  fclose(newRelocMap);
  newRelocMap = nullptr;
  freopen(Debug::DebugInfo::Get()->getRelocMapName().c_str(), "r+", relocMap);
  fseek(relocMap, 0, SEEK_END);

  okToRelocate = false;
  Treadmill::enqueue(std::move(s));
}

}

//////////////////////////////////////////////////////////////////////

bool relocateNewTranslation(TransLoc& loc,
                            CodeCache::View cache,
                            CGMeta& fixups,
                            TCA* adjust /* = nullptr */) {
  auto& mainCode = cache.main();
  auto& coldCode = cache.cold();
  auto& frozenCode = cache.frozen();

  CodeBlock dest;
  size_t asm_count{0};
  RelocationInfo rel;

  TCA mainStartRel, coldStartRel, frozenStartRel;

  const TCA mainStart   = loc.mainStart();
  const TCA coldStart   = loc.coldCodeStart();
  const TCA frozenStart = loc.frozenCodeStart();

  size_t mainSize   = loc.mainSize();
  size_t coldSize   = loc.coldSize();
  size_t frozenSize = loc.frozenSize();
  auto const pad = RuntimeOption::EvalReusableTCPadding;

  TCA mainEndRel   = loc.mainEnd();
  TCA coldEndRel   = loc.coldEnd();
  TCA frozenEndRel = loc.frozenEnd();

  if ((mainStartRel = (TCA)mainCode.allocInner(mainSize + pad))) {
    mainSize += pad;

    dest.init(mainStartRel, mainSize, "New Main");
    asm_count += relocate(rel, dest, mainStart, loc.mainEnd(), cache.main(),
                          fixups, nullptr, AreaIndex::Main);
    mainEndRel = dest.frontier();

    mainCode.free(loc.mainStart(), mainSize - pad);
  } else {
    mainStartRel = loc.mainStart();
    rel.recordRange(mainStart, loc.mainEnd(), mainStart, loc.mainEnd());
  }

  if ((frozenStartRel = (TCA)frozenCode.allocInner(frozenSize + pad))) {
    frozenSize += pad;

    dest.init(frozenStartRel + sizeof(uint32_t), frozenSize, "New Frozen");
    asm_count += relocate(rel, dest, frozenStart, loc.frozenEnd(),
                          cache.frozen(), fixups, nullptr, AreaIndex::Frozen);
    frozenEndRel = dest.frontier();

    frozenCode.free(loc.frozenStart(), frozenSize - pad);
  } else {
    frozenStartRel = loc.frozenStart();
    rel.recordRange(frozenStart, loc.frozenEnd(), frozenStart, loc.frozenEnd());
  }

  if (&coldCode != &frozenCode) {
    if ((coldStartRel = (TCA)coldCode.allocInner(coldSize + pad))) {
      coldSize += pad;

      dest.init(coldStartRel + sizeof(uint32_t), coldSize, "New Cold");
      asm_count += relocate(rel, dest, coldStart, loc.coldEnd(),
                            cache.cold(), fixups, nullptr, AreaIndex::Cold);
      coldEndRel = dest.frontier();

      coldCode.free(loc.coldStart(), coldSize - pad);
    } else {
      coldStartRel = loc.coldStart();
      rel.recordRange(coldStart, loc.coldEnd(), coldStart, loc.coldEnd());
    }
  } else {
    coldStartRel = frozenStartRel;
    coldEndRel = frozenEndRel;
    coldSize = frozenSize;
  }

  if (adjust) {
    if (auto newaddr = rel.adjustedAddressAfter(*adjust)) {
      *adjust = newaddr;
    }
  }

  if (asm_count) {
    adjustForRelocation(rel);
    adjustMetaDataForRelocation(rel, nullptr, fixups);
    adjustCodeForRelocation(rel, fixups);
  }

  if (debug) {
    auto clearRange = [](TCA start, TCA end) {
      CodeBlock cb;
      cb.init(start, end - start, "Dead code");

      CGMeta fixups;
      SCOPE_EXIT { assert(fixups.empty()); };

      DataBlock db;
      Vauto vasm { cb, cb, db, fixups };
      vasm.unit().padding = true;
    };

    if (mainStartRel != loc.mainStart()) {
      clearRange(loc.mainStart(), loc.mainEnd());
    }
    if (coldStartRel != loc.coldStart()) {
      clearRange(loc.coldStart(), loc.coldEnd());
    }
    if (frozenStartRel != loc.frozenStart() &&
        loc.frozenStart() != loc.coldStart()) {
      clearRange(loc.frozenStart(), loc.frozenEnd());
    }
  }

  uint32_t* coldSizePtr   = reinterpret_cast<uint32_t*>(coldStartRel);
  uint32_t* frozenSizePtr = reinterpret_cast<uint32_t*>(frozenStartRel);

  *coldSizePtr   = coldSize;
  *frozenSizePtr = frozenSize;

  loc.setMainStart(mainStartRel);
  loc.setColdStart(coldStartRel);
  loc.setFrozenStart(frozenStartRel);

  loc.setMainSize(mainSize);

  RelocationInfo relStubs;
  auto record = [&](TCA s, TCA e) { relStubs.recordRange(s, e, s, e); };

  record(mainStartRel, mainEndRel);
  if (coldStartRel != frozenStartRel) {
    record(coldStartRel + sizeof(uint32_t), coldEndRel);
  }

  relocateStubs(loc, frozenStartRel + sizeof(uint32_t), frozenEndRel, relStubs,
                cache, fixups);

  return
    mainStart != mainStartRel ||
    coldStart != coldStartRel ||
    frozenStart != frozenStartRel;
}

void liveRelocate(int time) {
  switch (arch()) {
  case Arch::X64:
  case Arch::PPC64:
    break;
  case Arch::ARM:
    // Live (Dynamic) Relocation is not supported on ARM until smashable
    // locations are tracked and rebuilt using debug info.
    return;
  }

  auto relocMap = Debug::DebugInfo::Get()->getRelocMap();
  if (!relocMap) return;

  auto codeLock = lockCode();
  auto metaLock = lockMetadata();
  if (!okToRelocate) return;

  SCOPE_EXIT { fseek(relocMap, 0, SEEK_END); };
  fseek(relocMap, 0, SEEK_SET);

  std::vector<TransRelocInfo> relocs;
  if (time == -2) {
    readRelocations(relocMap, nullptr, readRelocsIntoVector, &relocs);
    if (!relocs.size()) return;
  } else if (time == -1) {
    readRelocations(relocMap, nullptr, readRelocsIntoVector, &relocs);
    if (!relocs.size()) return;

    std::mt19937 g(42);
    std::shuffle(relocs.begin(), relocs.end(), g);

    unsigned new_size = g() % ((relocs.size() + 1) >> 1);
    new_size += (relocs.size() + 3) >> 2;
    assert(new_size > 0 && new_size <= relocs.size());

    relocs.resize(new_size);
  } else {
    int pid = getpid();
    auto relocResultsName = folly::sformat("/tmp/hhvm-reloc-{}.results", pid);
    FILE* relocResultsFile;
    if (!(relocResultsFile = fopen(relocResultsName.c_str(), "w+"))) {
      Logger::Error("Error creating relocation results file '%s'\n",
                    relocResultsName.c_str());
      return;
    }
    SCOPE_EXIT { fclose(relocResultsFile); };
    try {
      HPHP::hfsort::jitsort(pid, time, relocMap, relocResultsFile);
      fseek(relocResultsFile, 0, SEEK_SET);
      readRelocations(relocResultsFile, nullptr, readRelocsIntoVector, &relocs);
    } catch (...) {
      Logger::Error("LiveRelocate failed");
      return;
    }
  }

  CGMeta fixups;
  auto& hot = code().view(TransKind::Optimize).main();
  relocate(relocs, hot, fixups);

  // Nothing other than reusedStubs should have data, and those don't need any
  // processing for liveRelocate().
  fixups.reusedStubs.clear();
  always_assert(fixups.empty());
}

std::string
perfRelocMapInfo(TCA start, TCA /*end*/, TCA coldStart, TCA coldEnd, SrcKey sk,
                 int argNum,
                 const GrowableVector<IncomingBranch>& incomingBranchesIn,
                 CGMeta& fixups) {
  for (auto& stub : fixups.reusedStubs) {
    Debug::DebugInfo::Get()->recordRelocMap(stub, 0, "NewStub");
  }
  swap_trick(fixups.reusedStubs);

  TransRelocInfoHelper trih;
  trih.skInt = sk.toAtomicInt();
  trih.argNum = argNum;

  for (auto v : incomingBranchesIn) {
    trih.incomingBranches.emplace_back(v.getOpaque());
  }

  for (auto v : fixups.addressImmediates) {
    trih.addressImmediates.emplace_back(v - code().base());
  }

  for (auto v : fixups.codePointers) {
    trih.codePointers.emplace_back((uint64_t)v);
  }

  for (auto v : fixups.alignments) {
    trih.alignments.emplace_back(v.first - code().base(), v.second);
  }

  trih.coldRange = std::make_pair(uint32_t(coldStart - code().base()),
                                  uint32_t(coldEnd - code().base()));

  BlobEncoder blob;
  blob(trih);

  auto data = base64_encode(static_cast<const char*>(blob.data()), blob.size());
  std::string id;
  if (sk.valid()) {
    int fid = sk.funcID();
    if (&code().blockFor(start) == &code().prof()) {
      // use -ve ids for translations we don't want
      // to relocate (currently just profiling trs).
      fid = -fid;
    }
    id = folly::to<std::string>(static_cast<int64_t>(fid));
  } else {
    id = std::string("Stub");
  }

  return id + " " + data;
}

//////////////////////////////////////////////////////////////////////

void relocateTranslation(
  const IRUnit& unit,
  CodeBlock& main, CodeBlock& main_in, CodeAddress main_start,
  CodeBlock& cold, CodeBlock& cold_in, CodeAddress cold_start,
  CodeBlock& frozen, CodeAddress frozen_start,
  AsmInfo* ai, CGMeta& meta
) {
  auto const& bc_map = meta.bcMap;
  if (!bc_map.empty()) {
    TRACE(1, "bcmaps before relocation\n");
    for (UNUSED auto const& map : bc_map) {
      TRACE(1, "%s %-6d %p %p %p\n",
            map.md5.toString().c_str(),
            map.bcStart,
            map.aStart,
            map.acoldStart,
            map.afrozenStart);
    }
  }
  if (ai) printUnit(kRelocationLevel, unit, " before relocation ", ai);

  RelocationInfo rel;
  size_t asm_count{0};

  asm_count += relocate(rel, main_in,
                        main.base(), main.frontier(), main,
                        meta, nullptr, AreaIndex::Main);
  asm_count += relocate(rel, cold_in,
                        cold.base(), cold.frontier(), cold,
                        meta, nullptr, AreaIndex::Cold);

  TRACE(1, "asm %ld\n", asm_count);

  if (&frozen != &cold) {
    rel.recordRange(frozen_start, frozen.frontier(),
                    frozen_start, frozen.frontier());
  }
  adjustForRelocation(rel);
  adjustMetaDataForRelocation(rel, ai, meta);
  adjustCodeForRelocation(rel, meta);

  if (ai) {
    static int64_t mainDeltaTotal = 0, coldDeltaTotal = 0;
    int64_t mainDelta = (main_in.frontier() - main_start) -
                        (main.frontier() - main.base());
    int64_t coldDelta = (cold_in.frontier() - cold_start) -
                        (cold.frontier() - cold.base());

    mainDeltaTotal += mainDelta;
    coldDeltaTotal += coldDelta;

    if (HPHP::Trace::moduleEnabledRelease(HPHP::Trace::printir, 1)) {
      HPHP::Trace::traceRelease("main delta after relocation: "
                                "%" PRId64 " (%" PRId64 ")\n",
                                mainDelta, mainDeltaTotal);
      HPHP::Trace::traceRelease("cold delta after relocation: "
                                "%" PRId64 " (%" PRId64 ")\n",
                                coldDelta, coldDeltaTotal);
    }
  }

#ifndef NDEBUG
  auto& ip = meta.inProgressTailJumps;
  for (size_t i = 0; i < ip.size(); ++i) {
    const auto& ib = ip[i];
    assertx(!main.contains(ib.toSmash()));
    assertx(!cold.contains(ib.toSmash()));
  }
  memset(main.base(), 0xcc, main.frontier() - main.base());
  memset(cold.base(), 0xcc, cold.frontier() - cold.base());
  if (arch() == Arch::ARM) {
    __builtin___clear_cache(reinterpret_cast<char*>(main.base()),
                            reinterpret_cast<char*>(main.frontier()));
    __builtin___clear_cache(reinterpret_cast<char*>(cold.base()),
                            reinterpret_cast<char*>(cold.frontier()));
  }
#endif
}

//////////////////////////////////////////////////////////////////////

/*
 * If TC reuse is enabled, attempt to relocate the newly-emitted translation to
 * a hole reclaimed from dead code. Returns true if the translation was
 * relocated and false otherwise.
 */
void tryRelocateNewTranslation(SrcKey sk, TransLoc& loc,
                               CodeCache::View code, CGMeta& fixups) {
  if (!RuntimeOption::EvalEnableReusableTC) return;

  TCA UNUSED ms = loc.mainStart(), me = loc.mainEnd(),
             cs = loc.coldStart(), ce = loc.coldEnd(),
             fs = loc.frozenStart(), fe = loc.frozenEnd();

  auto const did_relocate = relocateNewTranslation(loc, code, fixups);

  if (did_relocate) {
    FTRACE_MOD(Trace::reusetc, 1,
               "Relocated translation for func {} (id = {})  @ sk({}) "
               "from M[{}, {}], C[{}, {}], F[{}, {}] to M[{}, {}] "
               "C[{}, {}] F[{}, {}]\n",
               sk.func()->fullName()->data(), sk.func()->getFuncId(),
               sk.offset(), ms, me, cs, ce, fs, fe, loc.mainStart(),
               loc.mainEnd(), loc.coldStart(), loc.coldEnd(),
               loc.frozenStart(), loc.frozenEnd());
  } else {
    FTRACE_MOD(Trace::reusetc, 1,
               "Created translation for func {} (id = {}) "
               " @ sk({}) at M[{}, {}], C[{}, {}], F[{}, {}]\n",
               sk.func()->fullName()->data(), sk.func()->getFuncId(),
               sk.offset(), ms, me, cs, ce, fs, fe);
  }
}

//////////////////////////////////////////////////////////////////////

}}}
