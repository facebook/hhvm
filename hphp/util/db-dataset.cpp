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

#include "hphp/util/db-dataset.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DBDataSet::DBDataSet()
  : m_fields(nullptr), m_rowCount(0), m_colCount(0),
    m_row(nullptr), m_lengths(nullptr) {
}

DBDataSet::~DBDataSet() {
  close();
}

void DBDataSet::addResult(MYSQL *conn, MYSQL_RES *result) {
  if (result == nullptr) return;

  int rowCount = mysql_num_rows(result);
  if (rowCount) {
    int fieldCount = (int)mysql_field_count(conn);
    if (m_colCount == 0) {
      m_colCount = fieldCount;
    } else {
      assert(m_colCount == fieldCount);
      if (m_colCount > fieldCount) {
        m_colCount = fieldCount; // in case we overflow m_row later
      }
    }

    m_rowCount += rowCount;
    m_results.push_back(result);
  } else {
    mysql_free_result(result);
  }
}

void DBDataSet::addDataSet(DBDataSet &ds) {
  if (ds.m_results.empty()) return;

  if (m_colCount == 0) {
    m_colCount = ds.m_colCount;
  } else {
    assert(m_colCount == ds.m_colCount);
  }

  m_rowCount += ds.m_rowCount;
  m_results.insert(m_results.end(), ds.m_results.begin(), ds.m_results.end());
  ds.m_results.clear();
  ds.close();
}

void DBDataSet::close() {
  for (m_iter = m_results.begin(); m_iter != m_results.end(); ++m_iter) {
    mysql_free_result(*m_iter);
  }
  m_results.clear();
  m_fields = nullptr;
  m_row = nullptr;
  m_lengths = nullptr;
  m_rowCount = 0;
  m_colCount = 0;
}

///////////////////////////////////////////////////////////////////////////////

int DBDataSet::getFieldIndex(const char *fieldName) {
  assert(fieldName && *fieldName);

  // without any results, cannot really resolve field names
  if (m_results.empty()) return -1;

  if (m_fields == nullptr) {
    m_fields = mysql_fetch_fields(m_results.front());
  }

  for (int i = 0; i < m_colCount; i++) {
    if (strcmp(m_fields[i].name, fieldName) == 0) {
      return i;
    }
  }
  return -1;
}

MYSQL_FIELD *DBDataSet::getFields() const {
  if (m_fields == nullptr && !m_results.empty()) {
    m_fields = mysql_fetch_fields(m_results.front());
  }
  return m_fields;
}

///////////////////////////////////////////////////////////////////////////////

void DBDataSet::moveFirst() {
  for (m_iter = m_results.begin(); m_iter != m_results.end(); ++m_iter) {
    if (*m_iter && mysql_num_rows(*m_iter) > 0) {
      mysql_data_seek(*m_iter, 0);
      m_row = mysql_fetch_row(*m_iter);
      m_lengths = mysql_fetch_lengths(*m_iter);
      assert(m_row);
      assert(m_lengths);
      return;
    }
  }
  m_row = nullptr;
  m_lengths = nullptr;
}

void DBDataSet::moveNext() {
  if (m_iter != m_results.end()) {
    if (*m_iter) {
      m_row = mysql_fetch_row(*m_iter);
      m_lengths = mysql_fetch_lengths(*m_iter);
      if (m_row) return;
    }
    for (++m_iter; m_iter != m_results.end(); ++m_iter) {
      if (*m_iter && mysql_num_rows(*m_iter) > 0) {
        mysql_data_seek(*m_iter, 0);
        m_row = mysql_fetch_row(*m_iter);
        m_lengths = mysql_fetch_lengths(*m_iter);
        assert(m_row);
        assert(m_lengths);
        return;
      }
    }
  }
  m_row = nullptr;
  m_lengths = nullptr;
}

const char *DBDataSet::getField(int field) const {
  assert(field >= 0 && field < m_colCount);
  assert(m_row);
  if (m_row && field >= 0 && field < m_colCount) {
    return m_row[field];
  }
  return nullptr;
}

int DBDataSet::getFieldLength(int field) const {
  assert(field >= 0 && field < m_colCount);
  assert(m_lengths);
  if (m_lengths && field >= 0 && field < m_colCount) {
    return m_lengths[field];
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
}
