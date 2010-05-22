/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
#include <runtime/eval/runtime/eval_frame_injection.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/ast/construct.h>
#include <runtime/eval/parser/parser.h>
#include <runtime/eval/runtime/code_coverage.h>
#include <runtime/base/runtime_option.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

String EvalFrameInjection::getFileName() {
  return m_file;
}

Array EvalFrameInjection::getArgs() {
  return m_env.getParams();
}

void EvalFrameInjection::SetLine(const Construct *c) {
  int line1 = c->loc()->line1;
  ThreadInfo::s_threadInfo->m_top->line = line1;
  if (RuntimeOption::RecordCodeCoverage) {
    int line0 = c->loc()->line1; // TODO: fix parser to record line0
    CodeCoverage::Record(c->loc()->file, line0, line1);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}

