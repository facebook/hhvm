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

#include "log_aggregator.h"
#include "process.h"
#include "db_query.h"
#include "stack_trace.h"
#include "exception.h"

using namespace std;

namespace HPHP {

int LogAggregator::MaxSampleItem = 100;
int LogAggregator::MaxSampleItemPerHost = 10;
int LogAggregator::HashStackSize = 7;
LogAggregator LogAggregator::TheLogAggregator;

///////////////////////////////////////////////////////////////////////////////

LogAggregator::LogAggregator() : m_groups(NULL) {
  m_host = Process::GetHostName();
  m_process = Process::GetProcessId();
  m_groups = new StringToGroupPtrMap();
}

void LogAggregator::setCodeRevision(const char *revision) {
  m_revision = revision ? revision : "";
}

///////////////////////////////////////////////////////////////////////////////

void LogAggregator::log(const StackTrace &st, const std::string &message) {
  const int MY_STACK_DEPTH = 4;
  std::string hash = st.hexEncode(MY_STACK_DEPTH,
                                  MY_STACK_DEPTH + HashStackSize);

  Lock lock(m_mutex);
  GroupPtr &group = (*m_groups)[hash];
  if (!group) {
    group = GroupPtr(new Group());
  }
  group->m_count++;

  if ((int)group->m_items.size() < MaxSampleItemPerHost) {
    Item *item = new Item();
    item->occurred = time(NULL);
    item->thread = Process::GetThreadId();
    item->stack = st.hexEncode(MY_STACK_DEPTH + HashStackSize);
    item->message = message;
    group->m_items.push_back(item);
  }
}

void LogAggregator::flush(std::ostream &out) {
  StringToGroupPtrMap *groups = m_groups;
  {
    Lock lock(m_mutex);
    if (m_groups->empty()) return;
    m_groups = new StringToGroupPtrMap();
  }

  struct tm now;
  time_t tnow = time(NULL);
  localtime_r(&tnow, &now);
  char snow[64];
  strftime(snow, sizeof(snow), "%D %T", &now);

  out << "========================================================\n";
  out << "Revision: " << m_revision << "\n";
  out << "Host: " << m_host << "\n";
  out << "Process: " << m_process << "\n";
  out << "Aggregated at " << snow << "\n";
  out << "========================================================\n";

  for (StringToGroupPtrMap::const_iterator iter = groups->begin();
       iter != groups->end(); ++iter) {
    const string &hash = iter->first;
    GroupPtr group = iter->second;

    out << "Stack " << hash << " occurred " << group->m_count << " time(s):\n";
    for (unsigned int i = 0; i < group->m_items.size(); i++) {
      Item *item = group->m_items[i];
      out << "  [" << item->thread << "] [" << item->stack << "] "
          << item->message << "\n";
    }
    out << "--------------------------------------------------------\n";
  }

  out << "\n";
  delete groups;
}

///////////////////////////////////////////////////////////////////////////////
// MySQL error storage

/**
 * Schema: Only two tables are needed:

   create table log_group (revision varchar(255) not null, hash varchar(255) not null, count int not null, translated text not null, primary key (revision, hash));
   create table log_item (revision varchar(255) not null, hash varchar(255) not null, item int not null, host varchar(255) not null, process bigint not null, occurred bigint not null, thread bigint not null, stack text not null, message text not null, translated text not null, primary key (revision, hash, item));

 */
void LogAggregator::flush(ServerDataPtr server) {
  StringToGroupPtrMap *groups = m_groups;
  {
    Lock lock(m_mutex);
    if (m_groups->empty()) return;
    m_groups = new StringToGroupPtrMap();
  }

  try {
    DBConn conn;
    conn.open(server);

    // update groups
    DBQueryPtr qGroup
      (new DBQuery(&conn, "INSERT INTO log_group (revision, hash, count)"));
    for (StringToGroupPtrMap::const_iterator iter = groups->begin();
         iter != groups->end(); ++iter) {
      const string &hash = iter->first;
      GroupPtr group = iter->second;
      qGroup->insert("'%s', '%s', %d", m_revision.c_str(), hash.c_str(),
                     group->m_count);

      DBQueryPtr q
        (new DBQuery(&conn, "INSERT INTO log_item"
                     " (revision, hash, item,"
                     "  host, process, occurred, thread, stack, message)"));
      for (unsigned int i = 0; i < group->m_items.size(); i++) {
        Item *item = group->m_items[i];
        int index = rand() % MaxSampleItem;
        q->insert("'%s','%s',%d,'%s',%p,%p,%p,'%s','%s'",
                  m_revision.c_str(), hash.c_str(), index, m_host.c_str(),
                  (long)m_process, (long)item->occurred, (long)item->thread,
                  item->stack.c_str(), item->message.c_str());
      }
      q->append(" ON DUPLICATE KEY UPDATE "
                "  host     = VALUES(host),"
                "  process  = VALUES(process),"
                "  occurred = VALUES(occurred),"
                "  thread   = VALUES(thread),"
                "  stack    = VALUES(stack),"
                "  message  = VALUES(message)");
      q->execute();
    }
    qGroup->append(" ON DUPLICATE KEY UPDATE count = count + VALUES(count)");
    qGroup->execute();

  } catch (Exception e) {
    Logger::Error("unable to upload errors: %s", e.getMessage().c_str());
  } catch (...) {
    Logger::Error("unable to upload errors: (unknown error)");
  }

  // we have to dump errors even if we were not able to upload them, because
  // we don't want to hold them in memory for very long time
  delete groups;
}

///////////////////////////////////////////////////////////////////////////////
}
