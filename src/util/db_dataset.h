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

#ifndef __DB_DATASET_H__
#define __DB_DATASET_H__

#include "dataset.h"

namespace HPHP {

DECLARE_BOOST_TYPES(DBDataSet);
///////////////////////////////////////////////////////////////////////////////

/**
 * A DataSet that wraps a result set directly from an SQL query.
 */
class DBDataSet : public DataSet {
 public:
  DBDataSet();
  virtual ~DBDataSet();

  /**
   * Internally called by DBConn::Execute() to prepare a DBDataSet.
   */
  void addResult(MYSQL *conn, MYSQL_RES *result);

  /**
   * Merge ds into this, and clear ds.
   */
  void addDataSet(DBDataSet &ds);

  /**
   * Implementing DataSet.
   */
  virtual void close();
  virtual int getRowCount() const { return m_rowCount;}
  virtual int getColCount() const { return m_colCount;}
  virtual int getFieldIndex(const char *fieldName);
  virtual MYSQL_FIELD *getFields() const;
  virtual void moveFirst();
  virtual MYSQL_ROW getRow() const { return m_row;}
  virtual void moveNext();
  virtual const char *getField(int field) const;
  virtual int getFieldLength(int field) const;

 private:
  DBDataSet(DBDataSet &ds) { ASSERT(false);} // no copy constructor

  typedef std::list<MYSQL_RES*> ResultList;
  ResultList m_results;
  mutable MYSQL_FIELD *m_fields;

  int m_rowCount;
  int m_colCount;

  ResultList::const_iterator m_iter;
  MYSQL_ROW m_row;
  unsigned long *m_lengths;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __DB_DATASET_H__
