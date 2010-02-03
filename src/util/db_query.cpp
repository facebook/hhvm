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

#include "db_query.h"
#include "db_conn.h"
#include "db_dataset.h"
#include "util.h"

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DBQuery::DBQuery(DBConn *conn, const char *sql, ...)
  : m_conn(conn), m_insert(false) {
  ASSERT(sql && *sql);

  char buf[1024];
  va_list ap;
  va_start(ap, sql);
  vsnprintf(buf, sizeof(buf), sql, ap);
  va_end(ap);
  m_base = buf;
}

///////////////////////////////////////////////////////////////////////////////

void DBQuery::filterBy(const char *fmt, Op op /* = And */) {
  ASSERT(fmt && *fmt);

  if (m_where.empty()) {
    m_where = " where ";
  } else {
    switch (op) {
    case And: m_where += " and "; break;
    case Or:  m_where += " or ";  break;
    default: break;
    }
  }
  m_where += fmt;
}

void DBQuery::filterBy(const char *fmt, const char *value, Op op /* = And */) {
  ASSERT(m_conn);

  string escaped;
  m_conn->escapeString(value, escaped);
  char *where = (char*)malloc(strlen(fmt) + escaped.size() - 1);
  sprintf(where, fmt, escaped.c_str());

  filterBy(where, op);
  free(where);
}

void DBQuery::filterBy(const char *fmt, const std::string &value,
                       Op op /* = And */) {
  filterBy(fmt, value.c_str(), op);
}

void DBQuery::filterBy(const char *fmt, int value, Op op /* = And */) {
  char *where = (char*)malloc(strlen(fmt) + 16);
  sprintf(where, fmt, value);

  filterBy(where, op);
  free(where);
}

void DBQuery::filterBy(const char *fmt, unsigned int value,
                       Op op /* = And */) {
  filterBy(fmt, (int)value, op);
}

void DBQuery::filterBy(const char *fmt, DBQueryFilterPtr filter,
                       Op op /* = And */) {
  ASSERT(!filter->isEmpty());
  ASSERT(!m_filter);

  m_filter = filter;
  filterBy(fmt, op);
}

void DBQuery::orderBy(const char *field, bool ascending /* = true */) {
  m_order += m_order.empty() ? " ORDER BY " : ",";
  m_order += field;
  if (!ascending) {
    m_order += " DESC";
  }
}

void DBQuery::limit(int count, int offset /* = 0 */) {
  m_limit = " LIMIT ";
  if (offset) {
    m_limit += boost::lexical_cast<string>(offset) + ", ";
  }
  m_limit += boost::lexical_cast<string>(count);
}

void DBQuery::insert(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  format(fmt, ap);
  va_end(ap);
  m_insert = true;
  m_values.push_back(m_format);
}

void DBQuery::append(const char *extra) {
  m_extra += (extra ? extra : "");
}

void DBQuery::setField(const char *fmt) {
  m_values.push_back(fmt);
}

void DBQuery::setField(const char *fmt, const char *value) {
  setField(fmt, value, strlen(value));
}

void DBQuery::setField(const char *fmt, const std::string &value) {
  setField(fmt, value.data(), value.length());
}

void DBQuery::setField(const char *fmt, const char *binary, int len) {
  ASSERT(m_conn);

  string escaped;
  m_conn->escapeString(binary, len, escaped);

  char *buffer = (char*)malloc(strlen(fmt) + escaped.size());
  if (!buffer) {
    throw bad_alloc();
  }
  sprintf(buffer, fmt, escaped.c_str());
  setField(buffer);
  free(buffer);
}

void DBQuery::setField(const char *fmt, int value) {
  setField(fmt, boost::lexical_cast<string>(value).c_str());
}

void DBQuery::setField(const char *fmt, unsigned int value) {
  setField(fmt, (int)value);
}

///////////////////////////////////////////////////////////////////////////////

int DBQuery::execute() {
  return execute(NULL);
}

int DBQuery::execute(DBDataSet &ds) {
  return execute(&ds);
}

int DBQuery::execute(DBDataSet *ds) {
  ASSERT(m_conn);
  ASSERT(m_conn->isOpened());

  int affected = 0;
  for (const char *sql = getFirstSql(); sql; sql = getNextSql()) {
    affected += m_conn->execute(sql, ds);
  }
  return affected;
}

int DBQuery::execute(int &result) {
  DBDataSet ds;
  int affected = execute(ds);
  result = 0;
  for (ds.moveFirst(); ds.getRow(); ds.moveNext()) {
    result += ds.getIntField(0);
  }
  return affected;
}

int DBQuery::execute(unsigned int &result) {
  DBDataSet ds;
  int affected = execute(ds);
  result = 0;
  for (ds.moveFirst(); ds.getRow(); ds.moveNext()) {
    result += ds.getUIntField(0);
  }
  return affected;
}

///////////////////////////////////////////////////////////////////////////////

const char *DBQuery::getFirstSql() {
  if (m_filter) {
    const char *where = m_filter->getFirst(m_where);
    ASSERT(where);
    return getSql(where);
  }
  return getSql(m_where.c_str());
}

const char *DBQuery::getNextSql() {
  if (m_filter) {
    const char *where = m_filter->getNext(m_where);
    if (where) return getSql(where);
  }
  return NULL;
}

const char *DBQuery::getSql(const char *where) {
  if (m_values.empty()) {
    m_sql = m_base + where + m_order + m_limit + m_extra;
  } else if (m_insert) {

    int total = m_base.size() + 8 + m_extra.size();
    for (unsigned int i = 0; i < m_values.size(); i++) {
      total += m_values[i].size() + 4;
    }
    m_sql.reserve(total);

    m_sql = m_base;
    m_sql += " VALUES ";
    for (unsigned int i = 0; i < m_values.size(); i++) {
      if (i > 0) m_sql += ", ";
      m_sql += "(";
      m_sql += m_values[i];
      m_sql += ")";
    }
    m_sql += m_extra;
  } else {
    m_sql = m_base;
    m_sql += " SET ";
    for (unsigned int i = 0; i < m_values.size(); i++) {
      if (i > 0) m_sql += ",";
      m_sql += m_values[i];
    }
    m_sql += where;
    m_sql += m_limit;
    m_sql += m_extra;
  }
  return m_sql.c_str();
}

///////////////////////////////////////////////////////////////////////////////

const char *DBQuery::format(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  format(fmt, ap);
  va_end(ap);
  return m_format.c_str();
}

const char *DBQuery::format(const char *fmt, va_list ap) {
  m_format = fmt;

  for (string::size_type pos = m_format.find('%');
       pos != string::npos && pos < m_format.length() - 1;
       pos = m_format.find('%', pos + 1)) {
    switch (m_format[pos+1]) {
    case 's':
      {
        ASSERT(m_conn);
        const char *value = va_arg(ap, const char *);
        string escaped;
        m_conn->escapeString(value, escaped);
        m_format.replace(pos, 2, escaped);
        pos += escaped.size();
      }
      break;
    case 'd':
      {
        int value = va_arg(ap, int);

        char buf[12];
        sprintf(buf, "%d", value);
        m_format.replace(pos, 2, buf);
        pos += strlen(buf);
      }
      break;
    case 'p':
      {
        long value = va_arg(ap, long);

        char buf[20];
        sprintf(buf, "%ld", value);
        m_format.replace(pos, 2, buf);
        pos += strlen(buf);
      }
      break;
    case '%':
      m_format.erase(pos, 1);
      break;
    default:
      ASSERT(false);
    }
  }

  return m_format.c_str();
}

std::string DBQuery::escapeFieldName(const char *fieldNameList) {
  ASSERT(fieldNameList);
  string ret = "`";
  ret += fieldNameList;
  ret += "`";
  Util::replaceAll(ret, ",", "`,`");
  return ret;
}

std::string DBQuery::escapeFieldName(const std::string &fieldNameList) {
  return escapeFieldName(fieldNameList.c_str());
}

///////////////////////////////////////////////////////////////////////////////
}
