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

#include "hphp/runtime/vm/jit/align-arm.h"

#include "hphp/util/data-block.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"

namespace HPHP { namespace jit { namespace arm {

///////////////////////////////////////////////////////////////////////////////

bool is_aligned(TCA frontier, Alignment alignment) {
  return true;
}

void align(CodeBlock& cb, Alignment alignment, AlignContext context,
           bool fixups /* = false */) {
  vixl::MacroAssembler a { cb };

  switch (alignment) {
    case Alignment::CacheLine:
    case Alignment::CacheLineRoundUp:
    case Alignment::JmpTarget:
      break;

    case Alignment::SmashCmpq:
      break;

    case Alignment::SmashMovq:
    case Alignment::SmashJmp:
      // Smashable movs and jmps are two instructions plus inline 64-bit data,
      // so they need to be 8-byte aligned.
      if (!cb.isFrontierAligned(8)) a.Nop();
      break;

    case Alignment::SmashCall:
    case Alignment::SmashJcc:
    case Alignment::SmashJccAndJmp:
      // Other smashable control flow instructions are three instructions plus
      // inline 64-bit data, so it needs to be one instruction off from 8-byte
      // alignment.
      if (cb.isFrontierAligned(8)) a.Nop();
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

}}}
