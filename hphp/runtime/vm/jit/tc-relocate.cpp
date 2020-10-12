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

namespace {

TRACE_SET_MOD(mcg);

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
      SCOPE_EXIT { assertx(fixups.empty()); };

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

//////////////////////////////////////////////////////////////////////

void relocateTranslation(
  const IRUnit* unit,
  CodeBlock& main, CodeBlock& main_in, CodeAddress main_start,
  CodeBlock& cold, CodeBlock& cold_in, CodeAddress cold_start,
  CodeBlock& frozen, CodeAddress frozen_start,
  AsmInfo* ai, CGMeta& meta
) {
  auto const& bc_map = meta.bcMap;
  if (!bc_map.empty()) {
    TRACE(1, "bcmaps before relocation\n");
    for (UNUSED auto const& map : bc_map) {
      TRACE(1, "%s %p %p %p\n",
            showShort(map.sk).c_str(),
            map.aStart,
            map.acoldStart,
            map.afrozenStart);
    }
  }
  if (ai && unit) printUnit(kRelocationLevel, *unit, " before relocation ", ai);

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
    main.sync();
    cold.sync();
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
