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

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/util/stack-trace.h"

namespace HPHP {

TRACE_SET_MOD(isame);

//////////////////////////////////////////////////////////////////////

bool isame_log(const StringData* input, const StringData* arg) {
  FTRACE(1, "isame collision {} != {}\n", input->slice(), arg->slice());
  auto const rate = RO::EvalIsameCollisionSampleRate;
  if (StructuredLog::coinflip(rate)) {
    StructuredLogEntry sample;
    sample.force_init = true;
    sample.setInt("sample_rate", rate);
    sample.setStr("event", "isame");
    sample.setStr("lhs", input->slice());
    sample.setStr("rhs", arg->slice());
    StackTrace st(StackTrace::Force{});
    sample.setStackTrace("stack", st);
    VMRegAnchor _(VMRegAnchor::Soft);
    if (regState() == VMRegState::CLEAN) {
      if (auto const ar = jit::findVMFrameForDebug()) {
        auto const frame = BTFrame::regular(ar, kInvalidOffset);
        auto const trace = createCrashBacktrace(frame, (jit::CTCA) 0 /* rip? */);
        auto const bt = stringify_backtrace(trace, true);
        std::vector<folly::StringPiece> btlines;
        for (auto s = bt.slice(); !s.empty();) {
          btlines.emplace_back(s.split_step('\n'));
        }
        sample.setVec("php_backtrace", btlines);
      }
    }
    StructuredLog::log("hhvm_isame_collisions", sample);
  }
  return true;
}

int istrcmp_log(const char* s1, const char* s2) {
  FTRACE(1, "isame collision {} != {}\n", s1, s2);
  auto const rate = RO::EvalIsameCollisionSampleRate;
  if (StructuredLog::coinflip(rate)) {
    StructuredLogEntry sample;
    sample.force_init = true;
    sample.setInt("sample_rate", rate);
    sample.setStr("event", "istrcmp");
    sample.setStr("lhs", s1);
    sample.setStr("rhs", s2);
    StackTrace st(StackTrace::Force{});
    sample.setStackTrace("stack", st);
    VMRegAnchor _(VMRegAnchor::Soft);
    if (regState() == VMRegState::CLEAN) {
      if (auto const ar = jit::findVMFrameForDebug()) {
        auto const frame = BTFrame::regular(ar, kInvalidOffset);
        auto const trace = createCrashBacktrace(frame, (jit::CTCA) 0 /* rip? */);
        auto const bt = stringify_backtrace(trace, true);
        std::vector<folly::StringPiece> btlines;
        for (auto s = bt.slice(); !s.empty();) {
          btlines.emplace_back(s.split_step('\n'));
        }
        sample.setVec("php_backtrace", btlines);
      }
    }
    StructuredLog::log("hhvm_isame_collisions", sample);
  }
  return 0;
}

}
