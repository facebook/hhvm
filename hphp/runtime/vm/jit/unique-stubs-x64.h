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

#ifndef incl_HPHP_JIT_UNIQUE_STUBS_X64_H_
#define incl_HPHP_JIT_UNIQUE_STUBS_X64_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct UniqueStubs;

///////////////////////////////////////////////////////////////////////////////

namespace x64 {

///////////////////////////////////////////////////////////////////////////////

/*
 * LLVM catch traces expect vmfp to be in rdx.
 */
inline RegSet syncForLLVMCatch(Vout& v) {
  v << copy{rvmfp(), reg::rdx};
  return RegSet{reg::rdx};
}

///////////////////////////////////////////////////////////////////////////////

TCA emitFunctionEnterHelper(CodeBlock& cb, UniqueStubs& us);
TCA emitFreeLocalsHelpers(CodeBlock& cb, UniqueStubs& us);
TCA emitCallToExit(CodeBlock& cb);
TCA emitEndCatchHelper(CodeBlock& cb, UniqueStubs& us);

void enterTCImpl(TCA start, ActRec* stashedAR);

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
