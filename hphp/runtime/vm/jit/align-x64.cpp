/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP { namespace jit { namespace x64 {

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * Targets of jmps on x64 must be aligned to 16 bytes.
 */
constexpr size_t kJmpTargetAlign = 16;

struct AlignImpl {
  static DECLARE_ALIGN_TABLE(s_table);

  static void pad(CodeBlock& cb, AlignContext context, size_t bytes) {
    X64Assembler a { cb };

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

DEFINE_ALIGN_TABLE(AlignImpl::s_table);

}

///////////////////////////////////////////////////////////////////////////////

bool is_aligned(TCA frontier, Alignment alignment) {
  return jit::is_aligned<AlignImpl>(frontier, alignment);
}

void align(CodeBlock& cb, Alignment alignment, AlignContext context,
           bool fixups /* = true */) {
  return jit::align<AlignImpl>(cb, alignment, context, fixups);
}

const AlignInfo& alignment_info(Alignment alignment) {
  auto const idx = static_cast<uint32_t>(alignment);
  bool const is_jccandjmp = alignment == Alignment::SmashJccAndJmp;

  return AlignImpl::s_table[is_jccandjmp ? idx + 2 : idx];
}

///////////////////////////////////////////////////////////////////////////////

}}}
