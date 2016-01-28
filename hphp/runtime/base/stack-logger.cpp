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

#include "hphp/runtime/base/stack-logger.h"

#ifndef FACEBOOK

// folly::symbolizer isn't supported on all the platforms that HHVM is, so we
// deliberately exclude it in our CMake setup, which makes this feature FB-only
// for now.

namespace HPHP {
void log_native_stack(const char* msg) {}
}

#else // FACEBOOK

#include <folly/experimental/symbolizer/Elf.h>
#include <folly/experimental/symbolizer/StackTrace.h>
#include <folly/experimental/symbolizer/Symbolizer.h>

#include "hphp/util/logger.h"

namespace HPHP {

using namespace folly::symbolizer;

/*
 * Symbolize frames under the assumption that they are defined in
 * /proc/self/exe without relocation.  Needed because folly::symbolizer assumes
 * it can find files via /proc/self/maps, but that's not true when we remap the
 * text section with huge pages.  Code mostly swiped from folly, but with
 * significant modifications.
 */
static void symbolize_huge_text(const uintptr_t* addresses,
                                SymbolizedFrame* frames,
                                size_t addressCount) {
  auto elfFile = std::make_shared<ElfFile>("/proc/self/exe");
  if (!elfFile) {
    return;
  }

  // See if any addresses are here
  auto base = elfFile->getBaseAddress();
  auto textSection = elfFile->getSectionByName(".text");
  auto from = textSection->sh_addr;
  auto to = from + textSection->sh_size;

  for (size_t i = 0; i < addressCount; ++i) {
    auto& frame = frames[i];
    if (frame.found) {
      continue;
    }

    uintptr_t address = addresses[i];

    if (from > address || address >= to) {
      continue;
    }

    // Found
    frame.found = true;

    // Undo relocation
    frame.set(elfFile, address - base);
  }
}

void log_native_stack(const char* msg) {
  constexpr size_t kMaxFrames = 128;

  uintptr_t addresses[kMaxFrames];
  auto nframes = getStackTrace(addresses, kMaxFrames);

  Symbolizer symbolizer;
  std::vector<SymbolizedFrame> frames(nframes);
  symbolize_huge_text(addresses, frames.data(), nframes);
  symbolizer.symbolize(addresses, frames.data(), nframes);

  StringSymbolizePrinter printer;
  printer.println(addresses, frames.data(), nframes);
  Logger::Error("%s\n\nC++ stack:\n%s",
                msg,
                printer.str().c_str());
}

}

#endif // FACEBOOK
