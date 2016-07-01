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

#ifndef incl_HPHP_PDO_PGSQL_H_
#define incl_HPHP_PDO_PGSQL_H_

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/ext/pdo/pdo_driver.h"

namespace HPHP {
struct PDOPgSql : PDODriver {
  PDOPgSql();
  req::ptr<PDOResource> createResourceImpl() override;
  req::ptr<PDOResource> createResource(
    const sp_PDOConnection& conn
  ) override;
};

long pdo_attr_lval(const Array& options, int opt, long defaultValue);

String pdo_attr_strval(const Array& options, int opt, const char *def);

enum {
  PDO_PGSQL_ATTR_DISABLE_NATIVE_PREPARED_STATEMENT = PDO_ATTR_DRIVER_SPECIFIC,
  PDO_PGSQL_ATTR_DISABLE_PREPARES,
};

const StaticString
  s_PGSQL_ATTR_DISABLE_NATIVE_PREPARED_STATEMENT(
    "PGSQL_ATTR_DISABLE_NATIVE_PREPARED_STATEMENT"
  ),
  s_PGSQL_ATTR_DISABLE_PREPARES("PGSQL_ATTR_DISABLE_PREPARES");
}
#endif
