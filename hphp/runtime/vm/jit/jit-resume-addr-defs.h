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

#include "hphp/runtime/vm/jit/jit-resume-addr.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"

namespace HPHP::jit {

///////////////////////////////////////////////////////////////////////////////

inline JitResumeAddr::operator bool() const {
  return handler != nullptr;
}

inline JitResumeAddr JitResumeAddr::none() {
  return {nullptr, nullptr};
}

inline JitResumeAddr JitResumeAddr::helper(TCA tca) {
  // FromInterp helpers can be used directly.
  assertx(tca == tc::ustubs().resumeHelperFromInterp ||
          tca == tc::ustubs().resumeHelperFuncEntryFromInterp ||
          tca == tc::ustubs().resumeHelperNoTranslateFromInterp ||
          tca == tc::ustubs().resumeHelperNoTranslateFuncEntryFromInterp ||
          tca == tc::ustubs().interpHelperFromInterp ||
          tca == tc::ustubs().interpHelperFuncEntryFromInterp ||
          tca == tc::ustubs().interpHelperNoTranslateFromInterp ||
          tca == tc::ustubs().interpHelperNoTranslateFuncEntryFromInterp);
  return {tca, nullptr};
}

inline JitResumeAddr JitResumeAddr::ret(TCA tca) {
  // Sync return registers before transferring the control to TC.
  return {tc::ustubs().interpToTCRet, tca};
}

inline JitResumeAddr JitResumeAddr::trans(TCA tca) {
  // Nothing needed, reenterTC already synced rvmfp() and rvmsp().
  return {tca, nullptr};
}

inline JitResumeAddr JitResumeAddr::transFuncEntry(TCA tca) {
  // Currently the same as trans().
  return {tca, nullptr};
}

///////////////////////////////////////////////////////////////////////////////

}
