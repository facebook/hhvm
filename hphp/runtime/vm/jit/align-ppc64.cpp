/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2015                                   |
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

#include "hphp/runtime/vm/jit/align-ppc64.h"

#include "hphp/runtime/vm/jit/align-internal.h"
#include "hphp/runtime/vm/jit/smashable-instr-ppc64.h"

#include "hphp/ppc64-asm/asm-ppc64.h"
#include "hphp/util/data-block.h"

#include <folly/Bits.h>

#include <utility>

namespace HPHP { namespace jit { namespace ppc64 {

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * Targets of jmps on ppc64 must be aligned to instruction.
 */
constexpr size_t kJmpTargetAlign = ppc64_asm::Assembler::kBytesPerInstr;

struct AlignImpl {
  static DECLARE_ALIGN_TABLE(s_table);

  static void pad(CodeBlock& cb, AlignContext context, size_t bytes) {
    ppc64_asm::Assembler a { cb };

    switch (context) {
      case AlignContext::Live:
        a.emitNop(bytes);
        return;

      case AlignContext::Dead:
        if (bytes > 4) {
          a.trap();
          bytes -= 4;
        }
        if (bytes > 0) {
          a.emitNop(bytes);
        }
        return;
    }
    not_reached();
  }
};

DEFINE_ALIGN_TABLE(AlignImpl::s_table);

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

bool is_aligned(TCA frontier, Alignment alignment) {
  return jit::is_aligned<AlignImpl>(frontier, alignment);
}

void align(CodeBlock& cb, Alignment alignment, AlignContext context,
           bool fixups /* = true */) {
  return jit::align<AlignImpl>(cb, alignment, context, fixups);
}

///////////////////////////////////////////////////////////////////////////////

}}}
