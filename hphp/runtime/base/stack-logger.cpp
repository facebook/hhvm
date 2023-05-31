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

#include "hphp/runtime/base/stack-logger.h"

#ifndef HHVM_FACEBOOK

// folly::symbolizer isn't supported on all the platforms that HHVM is, so we
// deliberately exclude it in our CMake setup, which makes this feature FB-only
// for now.

namespace HPHP {
void log_native_stack(const char* msg) {}
}

#else // FACEBOOK

#include <folly/experimental/symbolizer/StackTrace.h>
#include <folly/experimental/symbolizer/Symbolizer.h>

#include "hphp/util/logger.h"

namespace HPHP {

using namespace folly::symbolizer;

void log_native_stack(const char* msg) {
  constexpr size_t kMaxFrames = 128;

  uintptr_t addresses[kMaxFrames];
  auto nframes = getStackTrace(addresses, kMaxFrames);

  Symbolizer symbolizer;
  std::vector<SymbolizedFrame> frames(nframes);
  symbolizer.symbolize(addresses, frames.data(), nframes);

  StringSymbolizePrinter printer;
  printer.println(frames.data(), nframes);
  Logger::Error("%s\n\nC++ stack:\n%s",
                msg,
                printer.str().c_str());
}

}

#endif // FACEBOOK
