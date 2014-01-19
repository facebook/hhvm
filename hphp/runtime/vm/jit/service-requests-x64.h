/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP { namespace JIT { namespace X64 {

typedef JIT::X64Assembler Asm;

/*
 * emitServiceReqWork --
 *
 *   Call a translator service co-routine. The code emitted here
 *   reenters the enterTC loop, invoking the requested service. Control
 *   will be returned non-locally to the next logical instruction in
 *   the TC.
 *
 *   Return value is a destination; we emit the bulky service
 *   request code into astubs.
 *
 *   Returns a continuation that will run after the arguments have been
 *   emitted. This is gross, but is a partial workaround for the inability
 *   to capture argument packs in the version of gcc we're using.
 */
TCA emitServiceReqWork(Asm& as, TCA start, bool persist, SRFlags flags,
                       ServiceRequest req, const ServiceReqArgVec& argInfo);

/*
 * "cb" may be either the main section or stubs section.
 */
void emitBindSideExit(CodeBlock& cb, CodeBlock& stubs, JIT::ConditionCode cc,
                      SrcKey dest);
void emitBindJcc(CodeBlock& cb, CodeBlock& stubs, JIT::ConditionCode cc,
                 SrcKey dest);
void emitBindJmp(CodeBlock& cb, CodeBlock& stubs, SrcKey dest);

/*
 * Returns the amount by which rVmSp should be adjusted.
 */
int32_t emitBindCall(CodeBlock& mainCode, CodeBlock& stubsCode,
                     SrcKey srcKey, const Func* funcd, int numArgs);

}}}

#endif
