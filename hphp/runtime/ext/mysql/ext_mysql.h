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
#include "hphp/runtime/base/config.h"

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

  virtual void moduleLoad(const IniSetting::Map& ini, Hdf config) {
    Hdf mysql = config["MySQL"];
    ReadOnly = Config::GetBool(ini, mysql["ReadOnly"]);
#ifdef FACEBOOK
    Localize = Config::GetBool(ini, mysql["Localize"]);
#endif
    ConnectTimeout = Config::GetInt32(ini, mysql["ConnectTimeout"], 1000);
    ReadTimeout = Config::GetInt32(ini, mysql["ReadTimeout"], 60000);
    WaitTimeout = Config::GetInt32(ini, mysql["WaitTimeout"], -1);
    SlowQueryThreshold = Config::GetInt32(ini, mysql["SlowQueryThreshold"], 1000);
    KillOnTimeout = Config::GetBool(ini, mysql["KillOnTimeout"]);
    MaxRetryOpenOnFail = Config::GetInt32(ini, mysql["MaxRetryOpenOnFail"], 1);
    MaxRetryQueryOnFail = Config::GetInt32(ini, mysql["MaxRetryQueryOnFail"], 1);
    Socket = Config::GetString(ini, mysql["Socket"]);
    TypedResults = Config::GetBool(ini, mysql["TypedResults"], true);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.mysql.typed_results", &TypedResults);
  }

  void moduleInit();
};

extern mysqlExtension s_mysql_extension;

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(mysql_connect, const String& server = null_string,
                                     const String& username = null_string,
                                     const String& password = null_string,
                                     bool new_link = false,
                                     int client_flags = 0,
                                     int connect_timeout_ms = -1,
                                     int query_timeout_ms = -1);
Variant HHVM_FUNCTION(mysql_connect_with_db,
                      const String& server = null_string,
                      const String& username = null_string,
                      const String& password = null_string,
                      const String& database = null_string,
                      bool new_link = false,
                      int client_flags = 0,
                      int connect_timeout_ms = -1,
                      int query_timeout_ms = -1);
Variant HHVM_FUNCTION(mysql_pconnect, const String& server = null_string,
                      const String& username = null_string,
                      const String& password = null_string,
                      int client_flags = 0,
                      int connect_timeout_ms = -1,
                      int query_timeout_ms = -1);
Variant HHVM_FUNCTION(mysql_pconnect_with_db,
                      const String& server = null_string,
                      const String& username = null_string,
                      const String& password = null_string,
                      const String& database = null_string,
                      int client_flags = 0,
                      int connect_timeout_ms = -1,
                      int query_timeout_ms = -1);
bool HHVM_FUNCTION(mysql_set_timeout, int query_timeout_ms = -1,
                   const Variant& link_identifier = uninit_null());
String HHVM_FUNCTION(mysql_escape_string, const String& unescaped_string);
Variant HHVM_FUNCTION(mysql_real_escape_string, const String& unescaped_string,
                      const Variant& link_identifier = uninit_null());
String HHVM_FUNCTION(mysql_get_client_info);
bool HHVM_FUNCTION(mysql_set_charset, const String& charset,
                   const Variant& link_identifier = uninit_null());
bool HHVM_FUNCTION(mysql_ping,
                   const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_client_encoding,
                      const Variant& link_identifier = uninit_null());
bool HHVM_FUNCTION(mysql_close,
                   const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_errno,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_error,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_warning_count,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_get_host_info,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_get_proto_info,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_get_server_info,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_info,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_insert_id,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_stat,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_thread_id,
                      const Variant& link_identifier = uninit_null());
bool HHVM_FUNCTION(mysql_select_db, const String& db,
                   const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_affected_rows,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_query, const String& query,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_multi_query, const String& query,
                      const Variant& link_identifier = uninit_null());
int HHVM_FUNCTION(mysql_next_result,
                  const Variant& link_identifier = uninit_null());
bool HHVM_FUNCTION(mysql_more_results,
                   const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_fetch_result,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_unbuffered_query, const String& query,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_list_dbs,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_list_tables, const String& database,
                      const Variant& link_identifier = uninit_null());
Variant HHVM_FUNCTION(mysql_list_processes,
                      const Variant& link_identifier = uninit_null());

#ifdef FACEBOOK
Variant HHVM_FUNCTION(mysql_async_connect_start,
                      const String& server = null_string,
                      const String& username = null_string,
                      const String& password = null_string,
                      const String& database = null_string);
bool HHVM_FUNCTION(mysql_async_connect_completed,
                   const Variant& link_identifier);
bool HHVM_FUNCTION(mysql_async_query_start,
                   const String& query, const Variant& link_identifier);
Variant HHVM_FUNCTION(mysql_async_query_result,
                      const Variant& link_identifier);
bool HHVM_FUNCTION(mysql_async_query_completed, const Resource& result);
Variant HHVM_FUNCTION(mysql_async_fetch_array, const Resource& result,
                                               int result_type = 1);
Variant HHVM_FUNCTION(mysql_async_wait_actionable, const Array& items,
                                                   double timeout);
int64_t HHVM_FUNCTION(mysql_async_status, const Variant& link_identifier);
#endif

bool HHVM_FUNCTION(mysql_data_seek, const Resource& result, int row);
Variant HHVM_FUNCTION(mysql_fetch_array, const Resource& result,
                                         int result_type = 3);
Variant HHVM_FUNCTION(mysql_fetch_object, const Resource& result,
                      const String& class_name = "stdClass",
                      const Variant& params = uninit_null());
Variant HHVM_FUNCTION(mysql_fetch_lengths, const Resource& result);
Variant HHVM_FUNCTION(mysql_result, const Resource& result, int row,
                                    const Variant& field = 0);
Variant HHVM_FUNCTION(mysql_num_fields, const Resource& result);
Variant HHVM_FUNCTION(mysql_num_rows, const Resource& result);
bool HHVM_FUNCTION(mysql_free_result, const Resource& result);
Variant HHVM_FUNCTION(mysql_fetch_field, const Resource& result,
                                         int field = -1);
bool HHVM_FUNCTION(mysql_field_seek, const Resource& result, int field);
Variant HHVM_FUNCTION(mysql_field_name, const Resource& result, int field);
Variant HHVM_FUNCTION(mysql_field_table, const Resource& result, int field);
Variant HHVM_FUNCTION(mysql_field_len, const Resource& result, int field);
Variant HHVM_FUNCTION(mysql_field_type, const Resource& result, int field);
Variant HHVM_FUNCTION(mysql_field_flags, const Resource& result, int field);

///////////////////////////////////////////////////////////////////////////////
extern const int64_t k_ASYNC_OP_INVALID;
extern const int64_t k_ASYNC_OP_UNSET;
extern const int64_t k_ASYNC_OP_CONNECT;
extern const int64_t k_ASYNC_OP_QUERY;
extern const int64_t k_ASYNC_OP_FETCH_ROW;

}

#endif // incl_HPHP_EXT_MYSQL_H_
