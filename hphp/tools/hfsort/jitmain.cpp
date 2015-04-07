/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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


#include "hphp/tools/hfsort/hfutil.h"
#include "hphp/tools/hfsort/jitsort.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"

#include <folly/Format.h>
#include <folly/ScopeGuard.h>

#include <stdio.h>
#include <assert.h>
#include <zlib.h>
#include <ctype.h>

using HPHP::hfsort::error;

int main(int argc, char* argv[]) {
  int pid = 0;
  int time = 20;
  char tmp;

  try {

    HPHP::Logger::LogLevel = HPHP::Logger::LogVerbose;

    if (argc < 2 || argc > 3 ||
        sscanf(argv[1], "%d %c", &pid, &tmp) != 1 ||
        (argc == 3 && sscanf(argv[2], "%d %c", &time, &tmp) != 1)) {
      error(folly::sformat("Usage: {} <process-id> [<time>]\n",
                           argc > 0 ? argv[0] : "jitsort").c_str());
    }

    bool skipPerf = pid < 0;
    if (skipPerf) pid = -pid;

    auto perfSymFileName = folly::sformat("/tmp/hhvm-reloc-{}.map", pid);
    FILE* perfSymFile;
    if (!(perfSymFile = fopen(perfSymFileName.c_str(), "r"))) {
      error("Error opening perf data file\n");
    }
    SCOPE_EXIT { fclose(perfSymFile); };

    auto relocResultsName = folly::sformat("/tmp/hhvm-reloc-{}.results", pid);
    FILE* relocResultsFile;
    if (!(relocResultsFile = fopen(relocResultsName.c_str(), "w"))) {
      error("Error creating perf results file\n");
    }
    SCOPE_EXIT { fclose(relocResultsFile); };

    HPHP::hfsort::jitsort(skipPerf ? -pid : pid, time,
                          perfSymFile, relocResultsFile);
    printf("Output saved in %s\n", relocResultsName.c_str());
  } catch (...) {
    HPHP::Logger::Error("JitSort failed");
  }

  return 0;
}
