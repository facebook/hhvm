/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <runtime/ext/ext_output.h>
#include <runtime/base/runtime_option.h>
#include <util/lock.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static ReadWriteMutex s_loggers_mutex;
typedef std::map<std::string, FILE*> LoggerMap;
static LoggerMap s_loggers;

bool f_hphp_log(CStrRef filename, CStrRef message) {
  if (!RuntimeOption::EnableApplicationLog) {
    return false;
  }

  FILE *f = NULL;
  {
    ReadLock lock(s_loggers_mutex);
    LoggerMap::const_iterator iter = s_loggers.find(filename.data());
    if (iter != s_loggers.end()) {
      f = iter->second;
    }
  }
  if (f == NULL) {
    WriteLock lock(s_loggers_mutex);
    LoggerMap::const_iterator iter = s_loggers.find(filename.data());
    if (iter != s_loggers.end()) {
      f = iter->second;
    } else {
      if (filename.charAt(0) == '|') {
        f = popen(filename.data() + 1, "w");
      } else {
        f = fopen(filename.data(), "a");
      }
      if (f == NULL) {
        return false;
      }
      s_loggers[filename.data()] = f;
    }
  }
  bool ret = (fwrite(message.data(), message.size(), 1, f) == 1);
  if (ret) {
    fflush(f);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
