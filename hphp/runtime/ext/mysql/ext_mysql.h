/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_MYSQL_H_
#define incl_HPHP_EXT_MYSQL_H_

#include "folly/Optional.h"

#include "hphp/runtime/base/base-includes.h"
#include "mysql.h"
#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/base/persistent-resource-store.h"

#ifdef PHP_MYSQL_UNIX_SOCK_ADDR
#ifdef MYSQL_UNIX_ADDR
#undef MYSQL_UNIX_ADDR
#endif
#define MYSQL_UNIX_ADDR PHP_MYSQL_UNIX_SOCK_ADDR
#endif

namespace HPHP {

class mysqlExtension : public Extension {
public:
  mysqlExtension() : Extension("mysql", "1.0") {}

  // implementing IDebuggable
  virtual int  debuggerSupport() {
    return SupportInfo;
  }
  virtual void debuggerInfo(InfoVec &info) {
    int count = g_persistentResources->getMap("mysql::persistent_conns").size();
    Add(info, "Persistent", FormatNumber("%" PRId64, count));

    AddServerStats(info, "sql.conn"       );
    AddServerStats(info, "sql.reconn_new" );
    AddServerStats(info, "sql.reconn_ok"  );
    AddServerStats(info, "sql.reconn_old" );
    AddServerStats(info, "sql.query"      );
  }

  static bool ReadOnly;
#ifdef FACEBOOK
  static bool Localize;
#endif
  static int ConnectTimeout;
  static int ReadTimeout;
  static int WaitTimeout;
  static int SlowQueryThreshold;
  static bool KillOnTimeout;
  static int MaxRetryOpenOnFail;
  static int MaxRetryQueryOnFail;
  static std::string Socket;
  static bool TypedResults;

  virtual void moduleLoad(Hdf config) {
    Hdf mysql = config["MySQL"];
    ReadOnly = mysql["ReadOnly"].getBool();
#ifdef FACEBOOK
    Localize = mysql["Localize"].getBool();
#endif
    ConnectTimeout = mysql["ConnectTimeout"].getInt32(1000);
    ReadTimeout = mysql["ReadTimeout"].getInt32(60000);
    WaitTimeout = mysql["WaitTimeout"].getInt32(-1);
    SlowQueryThreshold = mysql["SlowQueryThreshold"].getInt32(1000);
    KillOnTimeout = mysql["KillOnTimeout"].getBool();
    MaxRetryOpenOnFail = mysql["MaxRetryOpenOnFail"].getInt32(1);
    MaxRetryQueryOnFail = mysql["MaxRetryQueryOnFail"].getInt32(1);
    Socket = mysql["Socket"].getString();
    TypedResults = mysql["TypedResults"].getBool(true);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.mysql.typed_results", &TypedResults);
  }
};

extern mysqlExtension s_mysql_extension;

///////////////////////////////////////////////////////////////////////////////
// connection functions

Variant f_mysql_connect(const String& server = null_string,
                        const String& username = null_string,
                        const String& password = null_string,
                        bool new_link = false,
                        int client_flags = 0,
                        int connect_timeout_ms = -1,
                        int query_timeout_ms = -1);
Variant f_mysql_pconnect(const String& server = null_string,
                         const String& username = null_string,
                         const String& password = null_string,
                         int client_flags = 0,
                         int connect_timeout_ms = -1,
                         int query_timeout_ms = -1);

Variant f_mysql_connect_with_db(const String& server = null_string,
                        const String& username = null_string,
                        const String& password = null_string,
                        const String& database = null_string,
                        bool new_link = false,
                        int client_flags = 0,
                        int connect_timeout_ms = -1,
                        int query_timeout_ms = -1);
Variant f_mysql_pconnect_with_db(const String& server = null_string,
                         const String& username = null_string,
                         const String& password = null_string,
                         const String& database = null_string,
                         int client_flags = 0,
                         int connect_timeout_ms = -1,
                         int query_timeout_ms = -1);

Variant f_mysql_async_connect_start(const String& server = null_string,
                                    const String& username = null_string,
                                    const String& password = null_string,
                                    const String& database = null_string);
bool f_mysql_async_connect_completed(const Variant& link_identifier);
bool f_mysql_async_query_start(const String& query, const Variant& link_identifier);
Variant f_mysql_async_query_result(const Variant& link_identifier);
bool f_mysql_async_query_completed(const Variant& result);
Variant f_mysql_async_fetch_array(const Variant& result, int result_type = 1);
Variant f_mysql_async_wait_actionable(const Variant& items, double timeout);
int64_t f_mysql_async_status(const Variant& link_identifier);

String f_mysql_escape_string(const String& unescaped_string);

Variant f_mysql_real_escape_string(const String& unescaped_string,
                                   const Variant& link_identifier = uninit_null());

String f_mysql_get_client_info();
Variant f_mysql_set_charset(const String& charset,
                                   const Variant& link_identifier = uninit_null());
Variant f_mysql_ping(const Variant& link_identifier = uninit_null());
Variant f_mysql_client_encoding(const Variant& link_identifier = uninit_null());
Variant f_mysql_close(const Variant& link_identifier = uninit_null());

Variant f_mysql_errno(const Variant& link_identifier = uninit_null());

Variant f_mysql_error(const Variant& link_identifier = uninit_null());

Variant f_mysql_warning_count(const Variant& link_identifier = uninit_null());

Variant f_mysql_get_host_info(const Variant& link_identifier = uninit_null());
Variant f_mysql_get_proto_info(const Variant& link_identifier = uninit_null());
Variant f_mysql_get_server_info(const Variant& link_identifier = uninit_null());
Variant f_mysql_info(const Variant& link_identifier = uninit_null());
Variant f_mysql_insert_id(const Variant& link_identifier = uninit_null());
Variant f_mysql_stat(const Variant& link_identifier = uninit_null());
Variant f_mysql_thread_id(const Variant& link_identifier = uninit_null());
Variant f_mysql_create_db(const String& db,
                                 const Variant& link_identifier = uninit_null());
Variant f_mysql_select_db(const String& db,
                                 const Variant& link_identifier = uninit_null());
Variant f_mysql_drop_db(const String& db,
                               const Variant& link_identifier = uninit_null());
Variant f_mysql_affected_rows(const Variant& link_identifier = uninit_null());

///////////////////////////////////////////////////////////////////////////////
// query functions

bool f_mysql_set_timeout(int query_timeout_ms = -1,
                         const Variant& link_identifier = uninit_null());

Variant f_mysql_query(const String& query, const Variant& link_identifier = uninit_null());
Variant f_mysql_multi_query(const String& query, const Variant& link_identifier = uninit_null());

int f_mysql_next_result(const Variant& link_identifier = uninit_null());

bool f_mysql_more_results(const Variant& link_identifier = uninit_null());

Variant f_mysql_fetch_result(const Variant& link_identifier = uninit_null());

Variant f_mysql_unbuffered_query(const String& query,
                                 const Variant& link_identifier = uninit_null());
Variant f_mysql_db_query(const String& database, const String& query,
                         const Variant& link_identifier = uninit_null());
Variant f_mysql_list_dbs(const Variant& link_identifier = uninit_null());

Variant f_mysql_list_tables(const String& database,
                            const Variant& link_identifier = uninit_null());
Variant f_mysql_list_fields(const String& database_name, const String& table_name,
                            const Variant& link_identifier = uninit_null());
Variant f_mysql_list_processes(const Variant& link_identifier = uninit_null());

///////////////////////////////////////////////////////////////////////////////
// row operations

bool f_mysql_data_seek(const Variant& result, int row);

Variant f_mysql_fetch_row(const Variant& result);

Variant f_mysql_fetch_assoc(const Variant& result);

Variant f_mysql_fetch_array(const Variant& result, int result_type = 3);

Variant f_mysql_fetch_lengths(const Variant& result);

Variant f_mysql_fetch_object(const Variant& result, const String& class_name = "stdClass",
                             const Array& params = uninit_null().toArray());

Variant f_mysql_result(const Variant& result, int row, const Variant& field = null_variant);

///////////////////////////////////////////////////////////////////////////////
// result functions

Variant f_mysql_db_name(const Variant& result, int row,
                               const Variant& field = null_variant);
Variant f_mysql_tablename(const Variant& result, int i);

Variant f_mysql_num_fields(const Variant& result);
Variant f_mysql_num_rows(const Variant& result);
Variant f_mysql_free_result(const Variant& result);

///////////////////////////////////////////////////////////////////////////////
// field info

Variant f_mysql_fetch_field(const Variant& result, int field = -1);
bool f_mysql_field_seek(const Variant& result, int field = 0);
Variant f_mysql_field_name(const Variant& result, int field = 0);
Variant f_mysql_field_table(const Variant& result, int field = 0);
Variant f_mysql_field_len(const Variant& result, int field = 0);
Variant f_mysql_field_type(const Variant& result, int field = 0);
Variant f_mysql_field_flags(const Variant& result, int field = 0);

///////////////////////////////////////////////////////////////////////////////
extern const int64_t k_ASYNC_OP_INVALID;
extern const int64_t k_ASYNC_OP_UNSET;
extern const int64_t k_ASYNC_OP_CONNECT;
extern const int64_t k_ASYNC_OP_QUERY;
extern const int64_t k_ASYNC_OP_FETCH_ROW;

}

#endif // incl_HPHP_EXT_MYSQL_H_
