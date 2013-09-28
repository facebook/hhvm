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

#ifndef incl_HPHP_DATASET_H_
#define incl_HPHP_DATASET_H_

#include "hphp/util/base.h"
#include "mysql.h"

namespace HPHP {

DECLARE_BOOST_TYPES(DataSet);
///////////////////////////////////////////////////////////////////////////////

/**
 * A class that stores a list of rows, so that one can iterate through them by
 *
 * int fieldIndex = ds.getFieldIndex("id");
 * for (ds.moveFirst(); ds.getRow(0); ds.moveNext()) {
 *   const char *value = ds.getField(0);
 *   unsigned int n = ds.getUIntField(fieldIndex);
 * }
 */
class DataSet {
 public:
  virtual ~DataSet() {}

  /**
   * Close dataset and free up resources.
   */
  virtual void close() = 0;

  /**
   * Informational functions.
   */
  virtual int getRowCount() const = 0;
  virtual int getColCount() const = 0;
  virtual int getFieldIndex(const char *fieldName) = 0;
  virtual MYSQL_FIELD *getFields() const = 0;

  /**
   * Iteration functions.
   */
  virtual void moveFirst() = 0;
  virtual MYSQL_ROW getRow() const = 0;
  virtual void moveNext() = 0;

  /**
   * Field value retrieval.
   */
  virtual const char *getField(int field) const = 0;
  virtual int getFieldLength(int field) const = 0;
  const char *getStringField(int field) const; // NULL will become ""
  int getIntField(int field) const;
  unsigned int getUIntField(int field) const;
  long long getInt64Field(int field) const;
  unsigned long long getUInt64Field(int field) const;

  /**
   * Returns a one-liner of all fields of current row.
   */
  std::string getRowString() const;

 private:
  /**
   * Helper function for getRowString().
   */
  static std::string escape(const char *s);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DATASET_H_
