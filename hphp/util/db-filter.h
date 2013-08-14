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

#ifndef incl_HPHP_DB_FILTER_H_
#define incl_HPHP_DB_FILTER_H_

#include "hphp/util/base.h"

namespace HPHP {

class DBConn;

DECLARE_BOOST_TYPES(DBQueryFilter);
DECLARE_BOOST_TYPES(DBInNumberFilter);
DECLARE_BOOST_TYPES(DBInStringFilter);
DECLARE_BOOST_TYPES(DBOrStringFilter);
///////////////////////////////////////////////////////////////////////////////

/**
 * Base class for filters handling where clauses like
 *
 *  "field in (1412, 1241, 124, ...)"
 *  "field in ('string1', 'string2', ...)"
 *  "(...) or (...) or (...) or ..."
 *
 * All of them need special trunking to avoid oversized packets.
 */
class DBQueryFilter {
 public:
  DBQueryFilter() {}
  virtual ~DBQueryFilter() {}

  virtual bool isEmpty() const = 0;
  virtual unsigned int getCount() const = 0;
  virtual const char *getFirst(const std::string &where) = 0;
  virtual const char *getNext(const std::string &where) = 0;

  static int MAX_COUNT;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Handle "in" clause with numbers. For example,
 *  "field in (1412, 1241, 124, ...)"
 */
class DBInNumberFilter : public DBQueryFilter {
 public:
  DBInNumberFilter();

  /**
   * Just keep adding numbers to the filter.
   */
  void add(unsigned int value) { m_values.insert(value); }
  void add(int value) { m_values.insert(value); }

  virtual bool isEmpty() const { return m_values.empty();}
  virtual unsigned int getCount() const { return m_values.size();}
  virtual const char *getFirst(const std::string &where);
  virtual const char *getNext(const std::string &where);

 protected:
  std::string m_filter;
  std::set<int> m_values;
  std::set<int>::const_iterator m_iter;

  const char *getFilter(const std::string &where);
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Handle "in" clause with strings. For example,
 *  "field in ('string1', 'string2', ...)"
 */
class DBInStringFilter : public DBQueryFilter {
 public:
  /**
   * We need a connection object to escape strings.
   */
  explicit DBInStringFilter(DBConn *conn);

  /**
   * Just keep adding strings to the filter.
   */
  void add(const char *value) { assert(value); m_values.insert(value); }
  void add(const std::string &value) { m_values.insert(value); }
  void add(const std::vector<std::string> &values) {
    m_values.insert(values.begin(), values.end());
  }

  virtual bool isEmpty() const { return m_values.empty();}
  virtual unsigned int getCount() const { return m_values.size();}
  virtual const char *getFirst(const std::string &where);
  virtual const char *getNext(const std::string &where);

 protected:
  DBConn *m_conn;
  std::string m_filter;
  std::set<std::string> m_values;
  std::set<std::string>::const_iterator m_iter;

  const char *getFilter(const std::string &where);
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Handle "or" clause with strings. For example,
 *  "(...) or (...) or (...) or ..."
 */
class DBOrStringFilter : public DBQueryFilter {
 public:
  DBOrStringFilter();

  /**
   * Please make sure all strings are escaped already.
   * Parentheses will be added for each of the strings. So
   *
   * f->Add("a = 1 and b = 2");
   * f->Add("a = 3 and b = 4");
   *
   * will result in "(a = 1 and b = 2) or (a = 3 and b = 4)".
   */
  void add(const char *value) { m_values.insert(value); }
  void add(const std::string &value) { m_values.insert(value); }

  virtual bool isEmpty() const { return m_values.empty();}
  virtual unsigned int getCount() const { return m_values.size();}
  virtual const char *getFirst(const std::string &where);
  virtual const char *getNext(const std::string &where);

 protected:
  std::string m_filter;
  std::set<std::string> m_values;
  std::set<std::string>::const_iterator m_iter;

  const char *getFilter(const std::string &where);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DB_FILTER_H_
