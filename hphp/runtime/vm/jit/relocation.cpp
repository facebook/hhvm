/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/relocation.h"

#include "hphp/util/trace.h"
#include "hphp/util/logger.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"

#include "hphp/tools/hfsort/jitsort.h"

#include <cstdio>
#include <vector>
#include <algorithm>

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

struct TransRelocInfo {
  TransRelocInfo() {}

  TransRelocInfo(TransRelocInfo&& other) = default;
  TransRelocInfo& operator=(TransRelocInfo&& other) = default;
  TransRelocInfo(const TransRelocInfo&) = delete;
  TransRelocInfo& operator=(const TransRelocInfo& other) = delete;
  SrcKey sk;
  int argNum;
  TCA start;
  TCA end;
  TCA coldStart;
  TCA coldEnd;
  GrowableVector<IncomingBranch> incomingBranches;
  CodeGenFixups fixups;
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
    LeaseHolder writer(Translator::WriteLease());
    if (!writer) {
      Treadmill::enqueue(std::move(*this));
      return;
    }

    for (auto& e : entries) {
      CodeBlock cb;
      cb.init(e.first, e.second - e.first, "relocated");
      X64Assembler a { cb };
      while (a.canEmit(2)) {
        a.ud2();
      }
      if (a.canEmit(1)) a.int3();
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
    x64::adjustForRelocation(rel, tri.start, tri.end);
    auto coldStart = tri.coldStart;
    if (&mcg->code.blockFor(coldStart) == &mcg->code.realFrozen()) {
      /*
       * This is a bit silly. If we were generating code into frozen,
       * and we also put stubs in frozen, and those stubs are now dead
       * the code in them isn't valid (we smashed the first few bytes
       * with a pointer and a size; see FreeStubList::StubNode).
       * So skip over any of those. Its ok to process the live ones
       * more than once.
       */
      auto it = deadStubs.lower_bound(tri.coldStart);
      while (it != deadStubs.end() && *it < tri.coldEnd) {
        x64::adjustForRelocation(rel, coldStart, *it);
        coldStart = *it + mcg->backEnd().reusableStubSize();
        ++it;
      }
    }
    x64::adjustForRelocation(rel, coldStart, tri.coldEnd);
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
  x64::adjustMetaDataForRelocation(rel, nullptr, tri.fixups);

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
  if (mcg->code.prof().contains(tri.start)) return;
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
  std::vector<std::pair<uint32_t, std::pair<int,int>>> alignFixups;

  template<class SerDe> void serde(SerDe& sd) {
    sd
      (skInt)
      (argNum)
      (coldRange)
      (incomingBranches)
      (addressImmediates)
      (codePointers)
      (alignFixups)
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
      tri.fixups.m_addressImmediates.insert(ai + code.base());
    }
    for (auto& cp : codePointers) {
      tri.fixups.m_codePointers.insert((TCA*)cp);
    }
    for (auto v : alignFixups) {
      tri.fixups.m_alignFixups.emplace(v.first + code.base(),
                                         v.second);
    }
    return tri;
  }
};

void relocateStubs(TransLoc& loc, TCA frozenStart, TCA frozenEnd,
                   RelocationInfo& rel, CodeCache& cache,
                   CodeGenFixups& fixups) {
  auto const stubSize = mcg->backEnd().reusableStubSize();

  for (auto addr : fixups.m_reusedStubs) {
    if (!loc.contains(addr)) continue;
    always_assert(frozenStart <= addr);

    CodeBlock dest;
    dest.init(cache.frozen().frontier(), stubSize, "New Stub");
    x64::relocate(rel, dest, addr, addr + stubSize, fixups, nullptr);
    cache.frozen().skip(stubSize);
    if (addr != frozenStart) {
      rel.recordRange(frozenStart, addr, frozenStart, addr);
    }
    frozenStart = addr + stubSize;
  }
  if (frozenStart != frozenEnd) {
    rel.recordRange(frozenStart, frozenEnd, frozenStart, frozenEnd);
  }

  x64::adjustForRelocation(rel);
  x64::adjustMetaDataForRelocation(rel, nullptr, fixups);
  x64::adjustCodeForRelocation(rel, fixups);
}

}

//////////////////////////////////////////////////////////////////////

void RelocationInfo::recordRange(TCA start, TCA end,
                                 TCA destStart, TCA destEnd) {
  m_srcRanges.emplace_back(start, end);
  m_dstRanges.emplace_back(destStart, destEnd);
  m_adjustedAddresses[start].second = destStart;
  m_adjustedAddresses[end].first = destEnd;
}

void RelocationInfo::recordAddress(TCA src, TCA dest, int range) {
  m_adjustedAddresses.emplace(src, std::make_pair(dest, dest + range));
}

TCA RelocationInfo::adjustedAddressAfter(TCA addr) const {
  auto it = m_adjustedAddresses.find(addr);
  if (it == m_adjustedAddresses.end()) return nullptr;

  return it->second.second;
}

TCA RelocationInfo::adjustedAddressBefore(TCA addr) const {
  auto it = m_adjustedAddresses.find(addr);
  if (it == m_adjustedAddresses.end()) return nullptr;

  return it->second.first;
}

void RelocationInfo::rewind(TCA start, TCA end) {
  if (m_srcRanges.size() && m_srcRanges.back().first == start) {
    assertx(m_dstRanges.size() == m_srcRanges.size());
    assertx(m_srcRanges.back().second == end);
    m_srcRanges.pop_back();
    m_dstRanges.pop_back();
  }
  auto it = m_adjustedAddresses.lower_bound(start);
  if (it == m_adjustedAddresses.end()) return;
  if (it->first == start) {
    // if it->second.first is set, start is also the end
    // of an existing region. Don't erase it in that case
    if (it->second.first) {
      it++->second.second = 0;
    } else {
      m_adjustedAddresses.erase(it++);
    }
  }
  while (it != m_adjustedAddresses.end() && it->first < end) {
    m_adjustedAddresses.erase(it++);
  }
  if (it == m_adjustedAddresses.end()) return;
  if (it->first == end) {
    // Similar to start above, end could be the start of an
    // existing region.
    if (it->second.second) {
      it++->second.first = 0;
    } else {
      m_adjustedAddresses.erase(it++);
    }
  }
}

//////////////////////////////////////////////////////////////////////

void liveRelocate(int time) {
  switch (arch()) {
  case Arch::X64:
    break;
  case Arch::ARM:
    // Relocation is not supported on arm.
    return;
  }

  if (RuntimeOption::EvalJitLLVM) {
    return;
  }

  auto relocMap = mcg->getDebugInfo()->getRelocMap();
  if (!relocMap) return;

  BlockingLeaseHolder writer(Translator::WriteLease());
  assert(writer);
  if (!okToRelocate) return;

  SCOPE_EXIT { fseek(relocMap, 0, SEEK_END); };
  fseek(relocMap, 0, SEEK_SET);

  std::vector<TransRelocInfo> relocs;
  if (time == -1) {
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

  auto& code = mcg->code;
  CodeCache::Selector cbSel(CodeCache::Selector::Args(code).hot(true));
  CodeBlock& hot = code.main();

  relocate(relocs, hot);
}

void recordPerfRelocMap(
    TCA start, TCA end,
    TCA coldStart, TCA coldEnd,
    SrcKey sk, int argNum,
    const GrowableVector<IncomingBranch> &incomingBranchesIn,
    CodeGenFixups& fixups) {
  String info = perfRelocMapInfo(start, end,
                                 coldStart, coldEnd,
                                 sk, argNum,
                                 incomingBranchesIn,
                                 fixups);
  mcg->getDebugInfo()->recordRelocMap(start, end, info);
}

String perfRelocMapInfo(
    TCA start, TCA end,
    TCA coldStart, TCA coldEnd,
    SrcKey sk, int argNum,
    const GrowableVector<IncomingBranch>& incomingBranchesIn,
    CodeGenFixups& fixups) {
  for (auto& stub : fixups.m_reusedStubs) {
    mcg->getDebugInfo()->recordRelocMap(stub, 0, "NewStub");
  }
  swap_trick(fixups.m_reusedStubs);

  TransRelocInfoHelper trih;
  trih.skInt = sk.toAtomicInt();
  trih.argNum = argNum;

  for (auto v : incomingBranchesIn) {
    trih.incomingBranches.emplace_back(v.getOpaque());
  }

  auto& code = mcg->code;

  for (auto v : fixups.m_addressImmediates) {
    trih.addressImmediates.emplace_back(v - code.base());
  }

  for (auto v : fixups.m_codePointers) {
    trih.codePointers.emplace_back((uint64_t)v);
  }

  for (auto v : fixups.m_alignFixups) {
    trih.alignFixups.emplace_back(v.first - code.base(), v.second);
  }

  trih.coldRange = std::make_pair(uint32_t(coldStart - code.base()),
                                  uint32_t(coldEnd - code.base()));

  BlobEncoder blob;
  blob(trih);

  String data = string_base64_encode(static_cast<const char*>(blob.data()),
                                     blob.size());
  String id;
  if (sk.valid()) {
    int fid = sk.funcID();
    if (&code.blockFor(start) == &code.prof()) {
      // use -ve ids for translations we don't want
      // to relocate (currently just profiling trs).
      fid = -fid;
    }
    id = String(static_cast<int64_t>(fid));
  } else {
    id = String("Stub");
  }

  return id + String(" ") + data;
}

void relocate(std::vector<TransRelocInfo>& relocs, CodeBlock& dest) {
  assert(Translator::WriteLease().amOwner());
  assert(!Func::s_treadmill);

  auto newRelocMapName = mcg->getDebugInfo()->getRelocMapName() + ".tmp";
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
  assert(mcg->cgFixups().m_alignFixups.empty());
  for (size_t sz = relocs.size(); num < sz; num++) {
    auto& reloc = relocs[num];
    if (ignoreEntry(reloc.sk)) continue;
    auto start DEBUG_ONLY = dest.frontier();
    try {
      x64::relocate(rel, dest,
                    reloc.start, reloc.end, reloc.fixups, nullptr);
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
  swap_trick(mcg->cgFixups().m_alignFixups);
  assert(mcg->cgFixups().empty());

  x64::adjustForRelocation(rel);

  for (size_t i = 0; i < num; i++) {
    if (!ignoreEntry(relocs[i].sk)) {
      x64::adjustMetaDataForRelocation(rel, nullptr, relocs[i].fixups);
    }
  }

  for (size_t i = 0; i < num; i++) {
    if (!ignoreEntry(relocs[i].sk)) {
      relocs[i].fixups.process_only(nullptr);
    }
  }

  // At this point, all the relocated code should be correct, and runable.
  // But eg if it has unlikely paths into cold code that has not been relocated,
  // then the cold code will still point back to the original, not the relocated
  // versions. Similarly reusable stubs will still point to the old code.
  // Since we can now execute the relocated code, its ok to start fixing these
  // things now.

  for (auto& it : mcg->tx().getSrcDB()) {
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

    x64::adjustCodeForRelocation(rel, reloc.fixups);
    reloc.fixups.clear();

    // fixup code references in the corresponding cold block to point
    // to the new code
    x64::adjustForRelocation(rel, reloc.coldStart, reloc.coldEnd);

    if (visitedFuncs.insert(f).second) {
      if (auto adjusted = rel.adjustedAddressAfter(f->getFuncBody())) {
        f->setFuncBody(adjusted);
      }
      int num = Func::getMaxNumPrologues(f->numParams());
      if (num < kNumFixedPrologues) num = kNumFixedPrologues;
      while (num--) {
        auto addr = f->getPrologue(num);
        if (auto adjusted = rel.adjustedAddressAfter(addr)) {
          f->setPrologue(num, adjusted);
        }
      }
    }
    if (reloc.end != reloc.start) {
      s.entries.emplace_back(reloc.start, reloc.end);
    }
  }

  auto relocMap = mcg->getDebugInfo()->getRelocMap();
  always_assert(relocMap);
  fseek(relocMap, 0, SEEK_SET);

  std::set<TCA> deadStubs;
  auto stub = (FreeStubList::StubNode*)mcg->freeStubList().peek();
  while (stub) {
    deadStubs.insert((TCA)stub);
    stub = stub->m_next;
  }

  auto param = PostProcessParam { rel, deadStubs, newRelocMap };
  std::set<TCA> liveStubs;
  readRelocations(relocMap, &liveStubs, postProcess, &param);

  // ensure that any reusable stubs are updated for the relocated code
  CodeGenFixups fixups;
  for (auto stub : liveStubs) {
    FTRACE(1, "Stub: 0x{:08x}\n", (uintptr_t)stub);
    fixups.m_reusedStubs.emplace_back(stub);
    always_assert(!rel.adjustedAddressAfter(stub));
    fprintf(newRelocMap, "%" PRIxPTR " 0 %s\n", uintptr_t(stub), "NewStub");
  }
  x64::adjustCodeForRelocation(rel, fixups);

  unlink(mcg->getDebugInfo()->getRelocMapName().c_str());
  rename(newRelocMapName.c_str(),
         mcg->getDebugInfo()->getRelocMapName().c_str());
  fclose(newRelocMap);
  newRelocMap = nullptr;
  freopen(mcg->getDebugInfo()->getRelocMapName().c_str(), "r+", relocMap);
  fseek(relocMap, 0, SEEK_END);

  okToRelocate = false;
  Treadmill::enqueue(std::move(s));
}

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
      String decoded = string_base64_decode(b64.c_str(), b64.size(), true);

      BlobDecoder blob(decoded.data(), decoded.size());
      TransRelocInfoHelper trih;
      blob(trih);

      TransRelocInfo tri(trih.toTRI(mcg->code));
      tri.start = reinterpret_cast<TCA>(addr);
      tri.end = reinterpret_cast<TCA>(end);
      x64::findFixups(tri.start, tri.end, tri.fixups);

      callback(std::move(tri), data);
    }
  }
}

//////////////////////////////////////////////////////////////////////

bool relocateNewTranslation(TransLoc& loc, CodeCache& cache,
                            TCA* adjust /* = nullptr */) {
  auto& mainCode = cache.main();
  auto& coldCode = cache.cold();
  auto& frozenCode = cache.frozen();

  CodeBlock dest;
  RelocationInfo rel;
  size_t asm_count{0};

  TCA mainStartRel, coldStartRel, frozenStartRel;

  TCA mainStart   = loc.mainStart();
  TCA coldStart   = loc.coldCodeStart();
  TCA frozenStart = loc.frozenCodeStart();

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
    asm_count += x64::relocate(rel, dest, mainStart, loc.mainEnd(),
                               mcg->cgFixups(), nullptr);
    mainEndRel = dest.frontier();

    mainCode.setFrontier(loc.mainStart());
  } else {
    mainStartRel = loc.mainStart();
    rel.recordRange(mainStart, loc.mainEnd(), mainStart, loc.mainEnd());
  }

  if ((frozenStartRel = (TCA)frozenCode.allocInner(frozenSize + pad))) {
    frozenSize += pad;

    dest.init(frozenStartRel + sizeof(uint32_t), frozenSize, "New Frozen");
    asm_count += x64::relocate(rel, dest, frozenStart, loc.frozenEnd(),
                               mcg->cgFixups(), nullptr);
    frozenEndRel = dest.frontier();

    frozenCode.setFrontier(loc.frozenStart());
  } else {
    frozenStartRel = loc.frozenStart();
    rel.recordRange(frozenStart, loc.frozenEnd(), frozenStart, loc.frozenEnd());
  }

  if (&coldCode != &frozenCode) {
    if ((coldStartRel = (TCA)coldCode.allocInner(coldSize + pad))) {
      coldSize += pad;

      dest.init(coldStartRel + sizeof(uint32_t), coldSize, "New Cold");
      asm_count += x64::relocate(rel, dest, coldStart, loc.coldEnd(),
                                 mcg->cgFixups(), nullptr);
      coldEndRel = dest.frontier();

      coldCode.setFrontier(loc.coldStart());
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
    x64::adjustForRelocation(rel);
    x64::adjustMetaDataForRelocation(rel, nullptr, mcg->cgFixups());
    x64::adjustCodeForRelocation(rel, mcg->cgFixups());
  }

  if (debug) {
    auto clearRange = [](TCA start, TCA end) {
      CodeBlock cb;
      cb.init(start, end - start, "Dead code");
      Asm a {cb};
      while (cb.available() >= 2) a.ud2();
      if (cb.available() > 0) a.int3();
      always_assert(!cb.available());
    };

    if (mainStartRel != loc.mainStart()) {
      clearRange(loc.mainStart(), loc.mainEnd());
    }
    if (coldStartRel != loc.coldStart()) {
      clearRange(loc.coldStart(), loc.coldEnd());
    }
    if (frozenStartRel != loc.frozenStart()) {
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
                cache, mcg->cgFixups());
  return asm_count != 0;
}

//////////////////////////////////////////////////////////////////////

}}
