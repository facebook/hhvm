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

#ifndef incl_HPHP_CRONOLOG_H_
#define incl_HPHP_CRONOLOG_H_

#include <atomic>
#include <string>
#include <cstdio>
#include <mutex>

#include <folly/portability/Stdlib.h>

#include "hphp/util/cronoutils.h"
#include "hphp/util/log-file-flusher.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Cronolog {
  Cronolog() = default;
  Cronolog(const Cronolog&) = delete;
  Cronolog& operator=(const Cronolog&) = delete;
  ~Cronolog() {
    if (m_prevFile) fclose(m_prevFile);
    if (m_file) fclose(m_file);
  }
  void setPeriodicity();
  FILE* getOutputFile();
  static void changeOwner(const std::string& username,
                          const std::string& symlink);
  PERIODICITY m_periodicity{UNKNOWN};
  PERIODICITY m_periodDelayUnits{UNKNOWN};
  int m_periodMultiple{1};
  int m_periodDelay{0};
  int m_useAmericanDateFormats{0};
  char m_fileName[PATH_MAX];
  char* m_startTime{nullptr};
  std::string m_template;
  std::string m_linkName;
  char* m_prevLinkName{nullptr};
  time_t m_timeOffset{0};
  time_t m_nextPeriod{0};
  FILE* m_prevFile{nullptr};
  FILE* m_file{nullptr};
  LogFileFlusher flusher;
  std::mutex m_mutex;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_CRONOLOG_H_
