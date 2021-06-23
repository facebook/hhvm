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

#include "hphp/runtime/vm/resumable.h"

#include "hphp/runtime/ext/asio/ext_async-generator.h"

#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

char* resumeModeShortName(ResumeMode resumeMode) {
  switch (resumeMode) {
    case ResumeMode::None: return "";
    case ResumeMode::Async: return "ra";
    case ResumeMode::GenIter: return "rg";
    default: not_reached();
  }
}

Optional<ResumeMode> nameToResumeMode(const std::string& name) {
  if (name == "") return ResumeMode::None;
  if (name == "ra") return ResumeMode::Async;
  if (name == "rg") return ResumeMode::GenIter;
  return std::nullopt;
}

ResumeMode resumeModeFromActRecImpl(ActRec* ar) {
  assertx(isResumed(ar));
  auto const func = ar->func();
  if (LIKELY(func->isAsyncFunction())) return ResumeMode::Async;
  if (func->isNonAsyncGenerator()) return ResumeMode::GenIter;
  auto const gen = frame_async_generator(ar);
  return gen->isEagerlyExecuted() ? ResumeMode::GenIter : ResumeMode::Async;
}

///////////////////////////////////////////////////////////////////////////////

}
