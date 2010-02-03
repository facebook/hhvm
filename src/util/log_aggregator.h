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

#ifndef __LOG_AGGREGATOR_H__
#define __LOG_AGGREGATOR_H__

#include "base.h"
#include "lock.h"
#include "db_conn.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * An in-memory log aggregator that prints summaries than individual lines of
 * logs.
 */
class StackTrace;
class LogAggregator {
public:
  static int MaxSampleItem;
  static int MaxSampleItemPerHost;
  static int HashStackSize;
  static LogAggregator TheLogAggregator;

public:
  LogAggregator();
  void setCodeRevision(const char *revision);

  void log(const StackTrace &st, const std::string &message);

  void flush(std::ostream &out);
  void flush(ServerDataPtr server);

private:
  Mutex m_mutex;
  std::string m_revision;
  std::string m_host;
  pid_t m_process;

  struct Item {
    time_t      occurred;
    pthread_t   thread;
    std::string stack;
    std::string message;
  };

  DECLARE_BOOST_TYPES(Group);
  class Group {
  public:
    Group() : m_count(0) {
    }
    ~Group() {
      for (unsigned int i = 0; i < m_items.size(); i++) delete m_items[i];
    }

    int m_count;
    std::vector<Item*> m_items; // samples
  };

  StringToGroupPtrMap *m_groups;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __LOG_AGGREGATOR_H__
