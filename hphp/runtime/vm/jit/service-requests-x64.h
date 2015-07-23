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
#ifndef incl_HPHP_JIT_SERVICE_REQUESTS_X64_H_
#define incl_HPHP_JIT_SERVICE_REQUESTS_X64_H_

#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit { namespace x64 {

/*
 * "cb" may be either the main section or frozen section.
 */
void emitBindJ(CodeBlock& cb,
               CodeBlock& frozen,
               jit::ConditionCode cc,
               SrcKey dest,
               FPInvOffset spOff,
               TransFlags trflags);

/*
 * Similar to the emitBindJ() series.  The address of the jmp is returned.
 */
TCA emitRetranslate(CodeBlock& cb,
                    CodeBlock& frozen,
                    jit::ConditionCode cc,
                    SrcKey dest,
                    folly::Optional<FPInvOffset> spOff,
                    TransFlags trflags);

/*
 * Emit a REQ_BIND_ADDR service request into `frozen', and return the
 * starting address for the service request.  This function takes into
 * account what the current block `cb' is while emitting the service
 * request.  That is, if `cb' is the same as `frozen', a jump is
 * emitted around the service request code.
 */
TCA emitBindAddr(CodeBlock& cb, CodeBlock& frozen, TCA* addr, SrcKey sk,
                 FPInvOffset spOff);

/*
 * Emit a REQ_BIND_JCC_FIRST service request in `frozen' and the
 * corresponding jumps to be smashed in `cb'.  `frozen' and `cb' are
 * allowed to be the same code block.
 */
void emitBindJmpccFirst(CodeBlock& cb,
                        CodeBlock& frozen,
                        ConditionCode cc,
                        SrcKey targetSk0,
                        SrcKey targetSk1,
                        FPInvOffset spOff);

// An intentionally funny-looking-in-core-dumps constant for uninitialized
// instruction pointers.
constexpr uint64_t kUninitializedRIP = 0xba5eba11acc01ade;

}}}

#endif
