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

#include "db_filter.h"
#include "db_conn.h"

using namespace std;

namespace HPHP {

int DBQueryFilter::MAX_COUNT = 500;

///////////////////////////////////////////////////////////////////////////////
// DBInNumberFilter

DBInNumberFilter::DBInNumberFilter() {
}

const char *DBInNumberFilter::getFirst(const std::string &where) {
  ASSERT(!m_values.empty());
  m_iter = m_values.begin();
  return getFilter(where);
}

const char *DBInNumberFilter::getNext(const std::string &where) {
  if (m_iter == m_values.end()) return NULL;
  return getFilter(where);
}

const char *DBInNumberFilter::getFilter(const std::string &where) {
  string values;
  for (int i = 0; i < MAX_COUNT; i++) {
    if (i > 0) values += ",";

    char buf[12];
    sprintf(buf, "%d", *m_iter);
    values += buf;

    ++m_iter;
    if (m_iter == m_values.end()) break;
  }

  string::size_type pos = where.find("%s");
  ASSERT(pos != string::npos);

  m_filter = where;
  m_filter.replace(pos, 2, values.c_str());
  return m_filter.c_str();
}

///////////////////////////////////////////////////////////////////////////////
// DBInStringFilter

DBInStringFilter::DBInStringFilter(DBConn *conn) : m_conn(conn) {
  //ASSERT(m_conn);
}

const char *DBInStringFilter::getFirst(const std::string &where) {
  ASSERT(!m_values.empty());
  m_iter = m_values.begin();
  return getFilter(where);
}

const char *DBInStringFilter::getNext(const std::string &where) {
  if (m_iter == m_values.end()) return NULL;
  return getFilter(where);
}

const char *DBInStringFilter::getFilter(const std::string &where) {
  string values;
  for (int i = 0; i < MAX_COUNT; i++) {
    if (i > 0) values += ",";

    values += "'";
    if (m_conn) {
      string escaped;
      m_conn->escapeString(m_iter->c_str(), escaped);
      values += escaped;
    } else {
      values += m_iter->c_str();
    }
    values += "'";

    ++m_iter;
    if (m_iter == m_values.end()) break;
  }

  string::size_type pos = where.find("%s");
  ASSERT(pos != string::npos);

  m_filter = where;
  m_filter.replace(pos, 2, values.c_str());
  return m_filter.c_str();
}

///////////////////////////////////////////////////////////////////////////////
// DBOrStringFilter

DBOrStringFilter::DBOrStringFilter() {
}

const char *DBOrStringFilter::getFirst(const std::string &where) {
  ASSERT(!m_values.empty());
  m_iter = m_values.begin();
  return getFilter(where);
}

const char *DBOrStringFilter::getNext(const std::string &where) {
  if (m_iter == m_values.end()) return NULL;
  return getFilter(where);
}

const char *DBOrStringFilter::getFilter(const std::string &where) {
  string values = "(";
  for (int i = 0; i < MAX_COUNT; i++) {
    if (i > 0) values += ") or (";

    values += *m_iter;

    ++m_iter;
    if (m_iter == m_values.end()) break;
  }
  values += ")";

  string::size_type pos = where.find("%s");
  ASSERT(pos != string::npos);

  m_filter = where;
  m_filter.replace(pos, 2, values.c_str());
  return m_filter.c_str();
}

///////////////////////////////////////////////////////////////////////////////
}
