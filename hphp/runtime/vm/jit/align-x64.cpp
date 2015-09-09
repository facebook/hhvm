/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/align-x64.h"

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/smashable-instr-x64.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

#include <folly/Bits.h>

#include <utility>

namespace HPHP { namespace jit { namespace x64 {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * Targets of jmps on x64 must be aligned to 16 bytes.
 */
constexpr size_t kJmpTargetAlign = 16;

/*
 * Alignment info table.
 */
constexpr AlignInfo s_aligns[] = {
  { cache_line_size(),  cache_line_size(),      0 },  // CacheLine
  { cache_line_size(),  cache_line_size() / 2,  0 },  // CacheLineRoundUp
  { kJmpTargetAlign,    kJmpTargetAlign,        0 },  // JmpTarget
  { cache_line_size(),  smashableMovqLen(),     kSmashMovqImmOff },
  { cache_line_size(),  smashableCmpqLen(),     kSmashCmpqImmOff },
  { cache_line_size(),  smashableCallLen(),     0 },
  { cache_line_size(),  smashableJmpLen(),      0 },
  { cache_line_size(),  smashableJccLen(),      0 },

  // Three entries for SmashJccAndJmp, one for each half and one for both
  // together.  The implementation below relies on this being the last
  // sequential enum value.
  { cache_line_size(),  smashableJccLen(),  0 },
  { cache_line_size(),  smashableJccLen() +
                        smashableJmpLen(),  smashableJccLen() },
  { cache_line_size(),  smashableJccLen() +
                        smashableJmpLen(),  0 }
};

bool is_aligned(TCA frontier, const AlignInfo& a) {
  assertx(a.nbytes <= a.align);
  assertx(folly::isPowTwo(a.align));

  auto const mask = a.align - 1;

  auto const ifrontier = reinterpret_cast<size_t>(frontier);
  auto const first_byte = ifrontier + a.offset;
  auto const last_byte  = ifrontier + a.nbytes - 1;

  return (first_byte & ~mask) == (last_byte & ~mask);
}

size_t align_gap(TCA frontier, const AlignInfo& a) {
  auto const ifrontier = reinterpret_cast<size_t>(frontier);
  auto const gap = a.align - ((a.align - 1) & (ifrontier + a.offset));

  return gap == a.align ? 0 : gap;
}

void pad_for_align(CodeBlock& cb, const AlignInfo& ali, AlignContext context) {
  X64Assembler a { cb };

  if (is_aligned(cb.frontier(), ali)) return;

  auto gap_sz = align_gap(cb.frontier(), ali);

  switch (context) {
    case AlignContext::Live:
      a.emitNop(gap_sz);
      return;

    case AlignContext::Dead:
      if (gap_sz > 2) {
        a.ud2();
        gap_sz -= 2;
      }
      if (gap_sz > 0) {
        a.emitInt3s(gap_sz);
      }
      return;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

bool is_aligned(TCA frontier, Alignment alignment) {
  auto const idx = static_cast<uint32_t>(alignment);
  bool const is_jccandjmp = alignment == Alignment::SmashJccAndJmp;

  return is_aligned(frontier, s_aligns[idx]) &&
    (!is_jccandjmp || is_aligned(frontier, s_aligns[idx + 1]));
}

void align(CodeBlock& cb, Alignment alignment, AlignContext context,
           bool fixups /* = true */) {
  auto const idx = static_cast<uint32_t>(alignment);
  bool const is_jccandjmp = alignment == Alignment::SmashJccAndJmp;

  pad_for_align(cb, s_aligns[idx], context);
  if (is_jccandjmp) pad_for_align(cb, s_aligns[idx + 1], context);

  assertx(is_aligned(cb.frontier(), s_aligns[idx]));
  if (is_jccandjmp) assertx(is_aligned(cb.frontier(), s_aligns[idx + 1]));

  if (fixups) {
    mcg->cgFixups().m_alignFixups.emplace(
      cb.frontier(),
      std::make_pair(alignment, context)
    );
  }
}

const AlignInfo& alignment_info(Alignment alignment) {
  auto const idx = static_cast<uint32_t>(alignment);
  bool const is_jccandjmp = alignment == Alignment::SmashJccAndJmp;

  return s_aligns[is_jccandjmp ? idx + 2 : idx];
}

///////////////////////////////////////////////////////////////////////////////

}}}
