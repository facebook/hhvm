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

#ifndef incl_HPHP_DB_QUERY_H_
#define incl_HPHP_DB_QUERY_H_

#include <stdarg.h>
#include "hphp/util/db-filter.h"

namespace HPHP {

class DBConn;
class DBDataSet;

DECLARE_BOOST_TYPES(DBQuery);
///////////////////////////////////////////////////////////////////////////////

/**
 * A class that helps construct SQL statements. When a filter or where clause
 * has a long list of items, this class will break them into optimal sizes of
 * several SQLs and run them separately and combine results into one. For
 * example,
 *
 * DBQuery q(&conn, "SELECT * FROM friend");
 * q.filterBy("user1 = %d", 12345);
 *
 * DBInNumberFilterPtr f(new DBInNumerFilter());
 * for (unsigned int i = 0; i < userIds.size(); i++) {
 *   f->add(userIds[i]);
 * }
 * q.filterBy("user2 in (%s)", filter);
 *
 * DBDataSet ds;
 * q.execute(ds);
 *
 * Note in the above example, if userIds.size() is very large, it will exceed
 * maximum allowed packet size of SQL statements. More importantly, according
 * to MySQL documentation, optimal query size is between 100 and 1000 when you
 * have a query like this that you can query in trunks and combine afterwards.
 */
class DBQuery {
 public:
  enum Op {
    None,
    And,
    Or,
  };

 public:
  /**
   * Constructor. Do NOT put any where clause here. Use FilterBy() instead.
   *
   *   DBQuery(&conn, "SELECT * FROM %s", tableName);
   */
  DBQuery(DBConn *conn, const char *sql, ...) ATTRIBUTE_PRINTF(3,4);

  /**
   * Composing where clause. String values will be escaped properly.
   * DBQueryFilterPtr can be used to pass in a list of items.
   */
  void filterBy(const char *fmt, Op op = And);
  void filterBy(const char *fmt, const char *value, Op op = And);
  void filterBy(const char *fmt, const std::string &value, Op op = And);
  void filterBy(const char *fmt, int value, Op op = And);
  void filterBy(const char *fmt, unsigned int value, Op op = And);
  void filterBy(const char *fmt, DBQueryFilterPtr filter, Op op = And);

  /**
   * Append ORDER BY clause.
   */
  void orderBy(const char *field, bool ascending = true);

  /**
   * Append LIMIT n or LIMIT m, n clause.
   */
  void limit(int count, int offset = 0);

  /**
   * Append (...) value list for an insertion.
   */
  void insert(const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);

  /**
   * Append extra SQL components. For example, ON DUPLICATE KEY UPDATE.
   */
  void append(const char *extra);

  /**
   * Append "field = value" to UPDATE statement.
   */
  void setField(const char *fmt);
  void setField(const char *fmt, const char *value);
  void setField(const char *fmt, const std::string &value);
  void setField(const char *fmt, const char *binary, int len);
  void setField(const char *fmt, int value);
  void setField(const char *fmt, unsigned int value);

  /**
   * Utility function that helps constructing more complex queries.
   * For example,
   *
   * q.filterBy(q.Format("(%s > 0 or %s = 2)", field1, field2));
   */
  const char *format(const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
  const char *format(const char *fmt, va_list ap) ATTRIBUTE_PRINTF(2,0);

  /**
   * Use "`" to escape all field names in a comma delimited field list.
   *
   * For example,
   *
   * DBQuery::escapeFieldName("from") == "`from`";
   * DBQuery::escapeFieldName("from,to") == "`from`,`to`";
   */
  static std::string escapeFieldName(const char *fieldNameList);
  static std::string escapeFieldName(const std::string &fieldNameList);

  /**
   * Run the query and return number of affected rows.
   */
  int execute();                     // ignore any results
  int execute(DBDataSet &ds);        // put results in ds
  int execute(DBDataSet *ds);        // put results in ds
  int execute(int &result);          // count(*) or single int col in result
  int execute(unsigned int &result); // put single unsigned int field in result

 private:
  DBConn *m_conn;

  std::string m_sql;
  std::string m_base;
  std::string m_where;
  std::string m_order;
  std::string m_limit;
  std::string m_extra;

  bool m_insert;
  std::vector<std::string> m_values;

  std::string m_format;

  DBQueryFilterPtr m_filter;

  const char *getFirstSql();
  const char *getNextSql();
  const char *getSql(const char *where);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DB_QUERY_H_
