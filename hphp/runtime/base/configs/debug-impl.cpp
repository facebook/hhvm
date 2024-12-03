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

#include "hphp/runtime/base/configs/debug-loader.h"

#include "hphp/util/configs/debug.h"

#include <sstream>
#include <unistd.h>

namespace HPHP::Cfg {

std::string DebugLoader::CoreDumpReportDirectoryDefault() {
#if defined(HPHP_OSS)
  return "/tmp";
#else
  return "/var/tmp/cores";
#endif
}

void DebugLoader::StackTraceFilenamePostProcess(std::string& val) {
  std::ostringstream stack_trace_stream;
    stack_trace_stream << Cfg::Debug::CoreDumpReportDirectory << "/stacktrace."
                       << (int64_t)getpid() << ".log";
  val = stack_trace_stream.str();
}

}
