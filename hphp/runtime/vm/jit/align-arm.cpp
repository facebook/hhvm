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

#include "hphp/runtime/vm/jit/align-arm.h"

#include "hphp/runtime/vm/jit/align-internal.h"
#include "hphp/runtime/vm/jit/smashable-instr-arm.h"

#include "hphp/util/data-block.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"

#include <folly/Bits.h>

#include <utility>

namespace HPHP { namespace jit { namespace arm {

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * Targets of jmps on arm must be aligned to instruction size
 */
constexpr size_t kJmpTargetAlign = vixl::kInstructionSize;

struct AlignImpl {
  static DECLARE_ALIGN_TABLE(s_table);

  static void pad(CodeBlock& cb, AlignContext context, size_t bytes) {
    vixl::MacroAssembler a { cb };
    auto const start = cb.toDestAddress(cb.frontier());

    switch (context) {
      case AlignContext::Live: {
        assert(((bytes & 3) == 0) && "alignment must be multiple of 4");
        for (; bytes > 0; bytes -= 4) {
          a.Nop();
        }
        auto const end = cb.toDestAddress(cb.frontier());
        __builtin___clear_cache(reinterpret_cast<char*>(start),
                                reinterpret_cast<char*>(end));
        return;
      }
      case AlignContext::Dead: {
        if (bytes > 4) {
          a.Brk();
          bytes -= 4;
        }
        auto const end = cb.toDestAddress(cb.frontier());
        __builtin___clear_cache(reinterpret_cast<char*>(start),
                                reinterpret_cast<char*>(end));
        if (bytes > 0) pad(cb, AlignContext::Live, bytes);
        return;
      }
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

void align(CodeBlock& cb, CGMeta* meta,
           Alignment alignment, AlignContext context) {
  return jit::align<AlignImpl>(cb, meta, alignment, context);
}

///////////////////////////////////////////////////////////////////////////////

}}}
