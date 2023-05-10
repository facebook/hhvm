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

#include "pdo_pgsql.h"
#include "pdo_pgsql_connection.h"
#include "pdo_pgsql_resource.h"

namespace HPHP {

PDOPgSql::PDOPgSql() : PDODriver("pgsql") {
  PQinitSSL(0);
}

req::ptr<PDOResource> PDOPgSql::createResourceImpl() {
  return req::make<PDOPgSqlResource>(std::make_shared<PDOPgSqlConnection>());
}

req::ptr<PDOResource> PDOPgSql::createResource(const sp_PDOConnection& conn) {
  return req::make<PDOPgSqlResource>(
    std::dynamic_pointer_cast<PDOPgSqlConnection>(conn)
  );
}

long pdo_attr_lval(const Array& options, int opt, long defaultValue){
  if(options.exists(opt)){
    return options[opt].toInt64();
  }
  return defaultValue;
}

String pdo_attr_strval(const Array& options, int opt, const char *def){
  if(options.exists(opt)){
    return options[opt].toString();
  }
  if(def){
    return def;
  }
  return String();
}

const StaticString s_PDO("PDO");
static struct PDOPGSQLExtension final : Extension {
  PDOPGSQLExtension() : Extension("pdo_pgsql", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}

  void moduleLoad(const IniSetting::Map& ini, Hdf hdf) override {
    HHVM_RCC_INT(PDO, PGSQL_ATTR_DISABLE_NATIVE_PREPARED_STATEMENT,
                 PDO_PGSQL_ATTR_DISABLE_NATIVE_PREPARED_STATEMENT);
    HHVM_RCC_INT(PDO, PGSQL_ATTR_DISABLE_PREPARES,
                 PDO_PGSQL_ATTR_DISABLE_PREPARES);
  }
} s_pdopgsql_extension;
}
