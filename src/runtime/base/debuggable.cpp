/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/debuggable.h>
#include <runtime/base/server/server_stats.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

std::string IDebuggable::FormatNumber(const char *fmt, int64 n) {
  char buf[64];
  snprintf(buf, sizeof(buf), fmt, n);
  return buf;
}

std::string IDebuggable::FormatSize(int64 size) {
  char buf[64];
  double n = size;
  if (n >= 1024) {
    n /= 1024;
    if (n >= 1024) {
      n /= 1024;
      if (n >= 1024) {
        n /= 1024;
        snprintf(buf, sizeof(buf), "%lld bytes (%.2fG)", size, n);
      } else {
        snprintf(buf, sizeof(buf), "%lld bytes (%.2fM)", size, n);
      }
    } else {
      snprintf(buf, sizeof(buf), "%lld bytes (%.2fk)", size, n);
    }
  } else {
    snprintf(buf, sizeof(buf), "%lld bytes", size);
  }
  return buf;
}

std::string IDebuggable::FormatTime(int64 milliSeconds) {
  char buf[64];
  double n = milliSeconds;
  if (n >= 1000) {
    n /= 1000;
    if (n >= 60) {
      n /= 60;
      if (n >= 60) {
        n /= 60;
        snprintf(buf, sizeof(buf), "%lld ms (%.2f hrs)", milliSeconds, n);
      } else {
        snprintf(buf, sizeof(buf), "%lld ms (%.2f min)", milliSeconds, n);
      }
    } else {
      snprintf(buf, sizeof(buf), "%lld ms (%.2f sec)", milliSeconds, n);
    }
  } else {
    snprintf(buf, sizeof(buf), "%lld ms", milliSeconds);
  }
  return buf;
}

///////////////////////////////////////////////////////////////////////////////

void IDebuggable::Add(InfoVec &out, const char *name,
                      const std::string &value) {
  out.push_back(InfoEntry(name, value));
}

void IDebuggable::AddServerStats(InfoVec &out, const char *name,
                                 const char *statsName /* = NULL */) {
  if (statsName == NULL) statsName = name;
  Add(out, name, FormatNumber("%lld", ServerStats::Get(statsName)));
}

///////////////////////////////////////////////////////////////////////////////
}
