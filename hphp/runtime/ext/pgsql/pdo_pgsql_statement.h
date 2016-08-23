/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2016 PocketRent Ltd and contributors                   |
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

#ifndef incl_HPHP_PDO_PGSQL_STATEMENT_H_
#define incl_HPHP_PDO_PGSQL_STATEMENT_H_

#include "hphp/runtime/ext/pdo/pdo_driver.h"
#include "pdo_pgsql_resource.h"
#include "pq.h"
#include "stdarg.h"

#define BOOLOID     16
#define BYTEAOID    17
#define INT8OID     20
#define INT2OID     21
#define INT4OID     23
#define TEXTOID     25
#define OIDOID      26

namespace HPHP {

struct PDOPgSqlStatement : PDOStatement {
  friend PDOPgSqlConnection;

  DECLARE_RESOURCE_ALLOCATION(PDOPgSqlStatement);

  PDOPgSqlStatement(req::ptr<PDOPgSqlResource> conn, PQ::Connection* server);
  virtual ~PDOPgSqlStatement();

  bool create(const String& sql, const Array &options);

  bool executer() override;
  bool fetcher(PDOFetchOrientation ori, long offset) override;

  bool describer(int colno) override;

  bool getColumnMeta(int64_t colno, Array &return_value) override;
  bool getColumn(int colno, Variant &value) override;

  bool paramHook(PDOBoundParam* param, PDOParamEvent event_type) override;

  bool cursorCloser() override;

  bool support(SupportedMethod method) override;

private:
  std::shared_ptr<PDOPgSqlConnection> m_conn;
  PQ::Connection* m_server;
  static unsigned long m_stmtNameCounter;
  static unsigned long m_cursorNameCounter;
  std::string m_stmtName;
  std::string m_resolvedQuery;
  std::string m_cursorName;
  std::string err_msg;
  PQ::Result m_result;
  bool m_isPrepared;
  bool m_hasParams;

  std::vector<Oid> param_types;
  std::vector<Variant> param_values;
  std::vector<int> param_lengths;
  std::vector<int> param_formats;

  std::vector<Oid> m_pgsql_column_types;

  long m_current_row;

  std::string oriToStr(PDOFetchOrientation ori, long offset, bool& success){
    success = true;
    switch(ori){
      case PDO_FETCH_ORI_NEXT:
        return std::string("NEXT");
      case PDO_FETCH_ORI_PRIOR:
        return std::string("BACKWARD");
      case PDO_FETCH_ORI_FIRST:
        return std::string("FIRST");
      case PDO_FETCH_ORI_LAST:
        return std::string("LAST");
      case PDO_FETCH_ORI_ABS:
        return std::string("ABSOLUTE ") + std::to_string(offset);
      case PDO_FETCH_ORI_REL:
        return std::string("RELATIVE ") + std::to_string(offset);
      default:
        success = false;
        return std::string();
    }
  }
};

}

#endif
