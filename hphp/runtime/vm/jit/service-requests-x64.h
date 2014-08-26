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
#ifndef incl_HPHP_JIT_SERVICE_REQUESTS_X64_H_
#define incl_HPHP_JIT_SERVICE_REQUESTS_X64_H_

#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit { namespace x64 {

/*
 * emitServiceReqWork --
 *
 *   Call a translator service co-routine. The code emitted here
 *   reenters the enterTC loop, invoking the requested service. Control
 *   will be returned non-locally to the next logical instruction in
 *   the TC.
 *
 *   Return value is a destination; we emit the bulky service
 *   request code into acold.
 *
 *   Returns a continuation that will run after the arguments have been
 *   emitted. This is gross, but is a partial workaround for the inability
 *   to capture argument packs in the version of gcc we're using.
 */
TCA emitServiceReqWork(CodeBlock& cb, TCA start, SRFlags flags,
                       ServiceRequest req, const ServiceReqArgVec& argInfo);

/*
 * "cb" may be either the main section or frozen section.
 */
void emitBindJmp(CodeBlock& cb, CodeBlock& frozen, SrcKey dest,
                 TransFlags trflags = TransFlags{});
void emitBindJcc(CodeBlock& cb, CodeBlock& frozen, jit::ConditionCode cc,
                 SrcKey dest);
void emitBindSideExit(CodeBlock& cb, CodeBlock& frozen, jit::ConditionCode cc,
                      SrcKey dest, TransFlags trflags = TransFlags{});

/*
 * Similar to the emitBindJ() series.  The address of the jmp is returned.
 */
TCA emitRetranslate(CodeBlock& cb, CodeBlock& frozen, jit::ConditionCode cc,
                    SrcKey dest, TransFlags trflags);

/*
 * Emits a REQ_BIND_CALL service request, and adjusts rVmSp after the call.
 */
void emitBindCall(CodeBlock& mainCode, CodeBlock& coldCode,
                  CodeBlock& frozenCode, SrcKey srcKey,
                  const Func* funcd, int numArgs);

}}}

#endif
