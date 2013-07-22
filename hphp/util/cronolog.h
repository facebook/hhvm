/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/cronoutils.h"
#include "hphp/util/lock.h"
#include "hphp/util/util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Cronolog);
class Cronolog {
public:
  Cronolog() :
    m_periodicity(UNKNOWN),
    m_periodDelayUnits(UNKNOWN),
    m_periodMultiple(1),
    m_periodDelay(0),
    m_useAmericanDateFormats(0),
    m_startTime(nullptr),
    m_prevLinkName(nullptr),
    m_timeOffset(0),
    m_nextPeriod(0),
    m_prevFile(nullptr),
    m_file(nullptr) {}
  ~Cronolog() {
    if (m_prevFile) fclose(m_prevFile);
    if (m_file) fclose(m_file);
  }
  void setPeriodicity();
  FILE *getOutputFile();
  static void changeOwner(const std::string &username,
                          const std::string &symlink);
public:
  PERIODICITY m_periodicity;
  PERIODICITY m_periodDelayUnits;
  int m_periodMultiple;
  int m_periodDelay;
  int m_useAmericanDateFormats;
  char m_fileName[PATH_MAX];
  char *m_startTime;
  std::string m_template;
  std::string m_linkName;
  char *m_prevLinkName;
  time_t m_timeOffset;
  time_t m_nextPeriod;
  FILE *m_prevFile;
  FILE *m_file;
  LogFileFlusher flusher;
  Mutex m_mutex;

private:
  Cronolog(const Cronolog &); // suppress
  Cronolog &operator=(const Cronolog &); // suppress
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_CRONOLOG_H_
