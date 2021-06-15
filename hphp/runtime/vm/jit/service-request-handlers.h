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

namespace jit { namespace svcreq {

/*
 * Handle a request to initially translate the code at the given current
 * location. Reached from translations at other SrcKeys when these translations
 * are unable or unwilling to continue due to insufficient type information or
 * to handle cold branches.
 *
 * The smash targets of a translate are stored in SrcRec::m_incomingBranches.
 */
TCA handleTranslate(Offset bcOff, SBInvOffset spOff) noexcept;

/*
 * Handle a request to retranslate the code at the given current location.
 * Reached from the last translation at the same SrcKey when no existing
 * translations support the incoming types.
 *
 * The smash targets of a retranslate are stored in SrcRec::m_tailFallbackJumps.
 */
TCA handleRetranslate(Offset bcOff, SBInvOffset spOff) noexcept;

/*
 * Handle a request to retranslate the current function, leveraging profiling
 * data to produce a set of larger, more optimized translations. Only used when
 * PGO is enabled. Execution will resume at `bcOff' whether or not retranslation
 * is successful.
 */
TCA handleRetranslateOpt(Offset bcOff, SBInvOffset spOff) noexcept;

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
 * Flags used by handleResume().
 */
struct ResumeFlags {
  ResumeFlags() : m_noTranslate{false}, m_interpFirst{false} {}

  /*
   * If `noTranslate` is true, no attempt to translate code will be made.
   * Preexisting translations will still be used.  This is used to avoid
   * repeated checks if we already know the attempt to translate will fail,
   * e.g. due to full TC.
   */
  ResumeFlags noTranslate(bool noTranslate = true) const {
    auto copy = *this;
    copy.m_noTranslate = noTranslate;
    return copy;
  }

  /*
   * If `interpFirst' is true, at least one basic block will be interpreted
   * before attempting to look up a translation.  This is necessary to ensure
   * forward progress in certain situations, such as hitting the translation
   * limit for a SrcKey.
   */
  ResumeFlags interpFirst(bool interpFirst = true) const {
    auto copy = *this;
    copy.m_interpFirst = interpFirst;
    return copy;
  }

  std::string show() const;

  union {
    uint8_t m_asByte;
    struct {
      bool m_noTranslate : 1;
      bool m_interpFirst : 1;
    };
  };
};

static_assert(sizeof(ResumeFlags) == 1, "ustubs pass m_asByte to handleResume");

/*
 * Look up (or create) and return the address of a translation for the current
 * VM location.
 *
 * If no translation can be found or created, execute code in the interpreter
 * until we find one, possibly throwing exceptions or reentering the VM.
 */
TCA handleResume(ResumeFlags flags);

/*
 * Look up (or create) the translation for the body of func.
 */
TCA getFuncBody(const Func* func);

}}}
