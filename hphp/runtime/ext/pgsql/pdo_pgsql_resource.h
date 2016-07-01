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

#ifndef incl_HPHP_PDO_PGSQL_RESOURCE_H_
#define incl_HPHP_PDO_PGSQL_RESOURCE_H_

#include "hphp/runtime/ext/pdo/pdo_driver.h"
#include "pdo_pgsql_connection.h"
#include "pq.h"
#include "stdarg.h"


namespace HPHP {
struct PDOPgSqlResource : PDOResource {
  explicit PDOPgSqlResource(std::shared_ptr<PDOPgSqlConnection> conn)
    : PDOResource(std::dynamic_pointer_cast<PDOConnection>(conn))
  {}

  std::shared_ptr<PDOPgSqlConnection> conn() const {
    return std::dynamic_pointer_cast<PDOPgSqlConnection>(m_conn);
  }
};
}

#endif
