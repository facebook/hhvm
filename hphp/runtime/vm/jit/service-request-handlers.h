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

#pragma once

#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {

struct ActRec;
struct SrcKey;

namespace jit {

struct ReqInfo;

namespace svcreq {

/*
 * Handle a service request.
 *
 * This often involves looking up or creating a translation, smashing a jmp
 * target or other address in the code, and returning the smashed-in value.
 * This address indicates where the caller should resume execution.
 */
TCA handleServiceRequest(ReqInfo& info) noexcept;

/*
 * Handle a situation where the translated code in the TC executes a return
 * for a frame that was pushed by the interpreter, i.e. there is no TCA to
 * return to.
 */
TCA handlePostInterpRet(uint32_t callOffAndFlags) noexcept;

/*
 * Handle a bindcall request---i.e., look up (or create) the appropriate func
 * prologue for `func' and `numArgs', then smash the call instruction
 * at `toSmash'.
 *
 * If we can't find or make a translation, may return fcallHelperThunk instead,
 * which uses C++ helpers to act like a prologue.
 */
TCA handleBindCall(TCA toSmash, Func* func, int32_t numArgs);

/*
 * Look up (or create) and return the address of a translation for the current
 * VM location.
 *
 * If no translation can be found or created, execute code in the interpreter
 * until we find one, possibly throwing exceptions or reentering the VM.
 *
 * If `interpFirst' is true, at least one basic block will be interpreted
 * before attempting to look up a translation.  This is necessary to ensure
 * forward progress in certain situations, such as hitting the translation
 * limit for a SrcKey.
 */
TCA handleResume(bool interpFirst);
TCA handleResumeNoTranslate(bool interpFirst);

/*
 * Look up (or create) the translation for the body of func.
 */
TCA getFuncBody(const Func* func);

}}}
