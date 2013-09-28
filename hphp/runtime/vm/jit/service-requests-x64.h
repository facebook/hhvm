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

typedef Transl::X64Assembler Asm;

void emitBindJcc(Asm& a, Asm& astubs, Transl::ConditionCode cc,
                 SrcKey dest, ServiceRequest req = REQ_BIND_JCC);
void emitBindJmp(Asm& a, Asm& astubs,
                 SrcKey dest, ServiceRequest req = REQ_BIND_JMP);

/*
 * Returns the amount by which rVmSp should be adjusted.
 */
int32_t emitBindCall(CodeBlock& mainCode, CodeBlock& stubsCode,
                     SrcKey srcKey, const Func* funcd, int numArgs);

}}}

#endif
