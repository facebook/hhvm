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
#include "hphp/runtime/vm/jit/vasm-gen.h"


#include "hphp/util/arch.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/logger.h"
#include "hphp/util/trace.h"

#include <algorithm>
#include <cstdio>
#include <vector>

namespace HPHP { namespace jit { namespace tc {

TRACE_SET_MOD(mcg);

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

}}}
