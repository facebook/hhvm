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

#ifndef incl_HPHP_PDO_PGSQL_CONNECTION_H_
#define incl_HPHP_PDO_PGSQL_CONNECTION_H_

#include "hphp/runtime/ext/pdo/pdo_driver.h"
#include "pq.h"

#define PHP_PDO_PGSQL_CONNECTION_FAILURE_SQLSTATE "08006"
namespace HPHP {
struct PDOPgSqlStatement;

struct PDOPgSqlConnection : PDOConnection {
  friend struct PDOPgSqlStatement;

  PDOPgSqlConnection();
  virtual ~PDOPgSqlConnection();

  bool create(const Array &options) override;

  int64_t doer(const String& sql) override;

  bool closer() override;

  bool begin() override;
  bool commit() override;
  bool rollback() override;

  bool checkLiveness() override;

  bool quoter(
    const String& input, String &quoted, PDOParamType paramtype
  ) override;

  bool support(SupportedMethod method) override;

  String lastId(const char *name) override;

  int getAttribute(int64_t attr, Variant &value) override;
  bool setAttribute(int64_t attr, const Variant &value) override;

  bool fetchErr(PDOStatement *stmt, Array &info) override;

  String pgsqlLOBCreate();

  bool preparer(
    const String& sql, sp_PDOStatement *stmt, const Variant& options
  ) override;

private:
  PQ::Connection* m_server;
  Oid pgoid;
  ExecStatusType m_lastExec;
  std::string err_msg;
  bool m_emulate_prepare;
  const char* sqlstate(PQ::Result& result);
  void handleError(
    PDOPgSqlStatement* stmt, const char* sqlState, const char* msg
  );
  bool transactionCommand(const char* command);
  void testConnection();
};
}

#endif
