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

#include "hphp/runtime/vm/jit/align-x64.h"

#include "hphp/runtime/vm/jit/align-internal.h"
#include "hphp/runtime/vm/jit/smashable-instr-x64.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

#include <folly/Bits.h>

#include <utility>

namespace HPHP::jit::x64 {

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * Targets of jmps on x64 must be aligned to 16 bytes.
 */
constexpr size_t kJmpTargetAlign = 16;

template <size_t SmashableAlignTo>
struct AlignImpl {

  static DECLARE_ALIGN_TABLE(s_table);

  static void pad(CodeBlock& cb, AlignContext context, size_t bytes) {
    X64Assembler a(cb);

    switch (context) {
      case AlignContext::Live:
        a.emitNop(bytes);
        return;

      case AlignContext::Dead:
        if (bytes > 2) {
          a.ud2();
          bytes -= 2;
        }
        if (bytes > 0) {
          a.emitInt3s(bytes);
        }
        return;
    }
    not_reached();
  }
};

// On AMD writes that crosses the 32 byte boundary are not atomic.
using AlignAMDImpl = AlignImpl<32>;
using AlignDefaultImpl = AlignImpl<cache_line_size()>;

}

///////////////////////////////////////////////////////////////////////////////

using IsAlignedFn = bool(*)(TCA, Alignment);

template <typename A>
bool is_aligned_impl(TCA frontier, Alignment alignment) {
  return jit::is_aligned<A>(frontier, alignment);
}

IsAlignedFn resolve_is_aligned() {
  if (folly::CpuId().vendor_amd()) return is_aligned_impl<AlignAMDImpl>;
  return is_aligned_impl<AlignDefaultImpl>;
}

static const IsAlignedFn s_is_aligned = resolve_is_aligned();

bool is_aligned(TCA frontier, Alignment alignment) {
  return s_is_aligned(frontier, alignment);
}

///////////////////////////////////////////////////////////////////////////////

using AlignFn = void(*)(CodeBlock&, CGMeta*, Alignment, AlignContext);

template <typename AlignImpl>
void align_impl(CodeBlock& cb, CGMeta* meta,
           Alignment alignment, AlignContext context) {
  return jit::align<AlignImpl>(cb, meta, alignment, context);
}

AlignFn resolve_align() {
  if (folly::CpuId().vendor_amd()) return align_impl<AlignAMDImpl>;
  return align_impl<AlignDefaultImpl>;
}

static const AlignFn s_align = resolve_align();

void align(CodeBlock& cb, CGMeta* meta,
           Alignment alignment, AlignContext context) {
  return s_align(cb, meta, alignment, context);
}

///////////////////////////////////////////////////////////////////////////////

using AlignmentInfoFn = const AlignInfo&(*)(Alignment);

template <typename AlignImpl>
const AlignInfo& alignment_info_impl(Alignment alignment) {
  auto const idx = static_cast<uint32_t>(alignment);

  return AlignImpl::s_table[idx];
}

AlignmentInfoFn resolve_alignment_info() {
  if (folly::CpuId().vendor_amd()) return alignment_info_impl<AlignAMDImpl>;
  return alignment_info_impl<AlignDefaultImpl>;
}

static const AlignmentInfoFn s_alignment_info = resolve_alignment_info();

const AlignInfo& alignment_info(Alignment alignment) {
  return s_alignment_info(alignment);
}

///////////////////////////////////////////////////////////////////////////////

}
