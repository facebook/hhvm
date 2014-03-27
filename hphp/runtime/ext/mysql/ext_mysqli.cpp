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

#include "hphp/runtime/base/base-includes.h"
#include <vector>
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/mysql/mysql_common.h"
#include "hphp/util/logger.h"

#include "mysql.h"

namespace HPHP {

const StaticString
  s_mysqli("mysqli"),
  s_connection("__connection"),
  s_link("__link"),
  s_result("__result"),
  s_stmt("__stmt"),
  s_charset("charset"),
  s_collation("collation"),
  s_dir("dir"),
  s_min_length("min_length"),
  s_max_length("max_length"),
  s_number("number"),
  s_state("state"),
  s_comment("comment"),
  s_param_count("param_count"),
  s_field_count("field_count"),
  s_mysqli_driver("mysqli_driver"),
  s_mysqli_result("mysqli_result"),
  s_mysqli_sql_exception("mysqli_sql_exception"),
  s_mysqli_stmt("mysqli_stmt"),
  s_mysqli_warning("mysqli_warning"),
  s_persistent_prefix("p:"),
  s_def("def");

//////////////////////////////////////////////////////////////////////////////
// helper
static Resource get_connection_resource(Object obj) {
  auto res = obj->o_realProp(s_connection, ObjectData::RealPropUnchecked,
                             s_mysqli.get());
  if (!res || !res->isResource()) {
    return null_resource;
  }

  return res->toResource();
}

static MySQL *get_connection(Object obj) {
  auto res = get_connection_resource(obj);
  return res.getTyped<MySQL>(true, false);
}

static MySQLStmt *getStmt(Object obj) {
  auto res = obj->o_realProp(s_stmt, ObjectData::RealPropUnchecked,
                             s_mysqli_stmt.get());
  assert(res->isResource());

  auto stmt = res->asResRef().getTyped<MySQLStmt>(false, false);
  assert(stmt);

  return stmt;
}

static MySQLResult *getResult(Object obj) {
  auto res = obj->o_realProp(s_result, ObjectData::RealPropUnchecked,
                             s_mysqli_result.get());
  if (!res || !res->isResource()) {
    return nullptr;
  }

  return res->toResource().getTyped<MySQLResult>(true, false);
}

static TypedValue* bind_param_helper(ObjectData* obj, ActRec* ar,
                                     int start_index) {
  String types(getArg<KindOfString>(ar, start_index));

  int type_size = types.size();

  if (type_size < 1) {
    raise_warning("Invalid type or no types specified");
    return arReturn(ar, false);
  }
  if (type_size != ar->numArgs() - 1 - start_index) {
    raise_warning("Number of elements in type definition string doesn't match "
                  "number of bind variables");
    return arReturn(ar, false);
  }
  if (type_size != obj->o_get(s_param_count.get()).toInt64()) {
    raise_warning("Number of variables doesn't match number of parameters in "
                  "prepared statement");
    return arReturn(ar, false);
  }

  std::vector<Variant*> vars;
  for (int i = 0; i < type_size; i++) {
    char t = types[i];
    if (t != 'i' && t != 'd' && t != 's' && t != 'b') {
      raise_warning("Undefined fieldtype %c (parameter %d)", types[i],
                    i + 2 + start_index);
      return arReturn(ar, false);
    }
    vars.push_back(&getArg<KindOfRef>(ar, i + 1 + start_index));
  }

  return arReturn(ar, getStmt(obj)->bind_param(types, vars));
}

static TypedValue* bind_result_helper(ObjectData* obj, ActRec* ar,
                                      int start_index) {
  int64_t fields = obj->o_get(s_field_count.get()).toInt64();
  if (ar->numArgs() - start_index != fields) {
    raise_warning("Number of bind variables doesn't match number of fields in "
                  "prepared statement");
    return arReturn(ar, false);
  }

  std::vector<Variant*> vars;
  for (int i = start_index; i < ar->numArgs(); i++) {
    vars.push_back(&getArg<KindOfRef>(ar, i));
  }

  return arReturn(ar, getStmt(obj)->bind_result(vars));
}

//////////////////////////////////////////////////////////////////////////////
// class mysqli

#define VALIDATE_CONN(conn, state)                                             \
  if (!conn || (state && (int64_t)conn->getState() < state)) {                 \
    raise_warning("invalid object or resource mysqli");                        \
    return init_null();                                                        \
  }

#define VALIDATE_CONN_CONNECTED(conn) VALIDATE_CONN(conn, MySQLState::CONNECTED)

#define VALIDATE_RESOURCE(res, state)                                          \
  MySQL* conn = res.getTyped<MySQL>(true, false);                              \
  VALIDATE_CONN(conn, state)

static Variant HHVM_METHOD(mysqli, autocommit, bool mode) {
  auto conn = get_connection(this_);
  VALIDATE_CONN_CONNECTED(conn);
  return !mysql_autocommit(conn->get(), (my_bool)mode);
}

static Variant HHVM_METHOD(mysqli, change_user, const String& user,
                           const String& password, const String& database) {
  auto conn = get_connection(this_);
  VALIDATE_CONN_CONNECTED(conn);
  return !mysql_change_user(conn->get(), user.c_str(), password.c_str(),
                            database.c_str());
}

static Variant HHVM_METHOD(mysqli, character_set_name) {
  auto conn = get_connection(this_);
  VALIDATE_CONN_CONNECTED(conn);
  return String(mysql_character_set_name(conn->get()), CopyString);
}

static Variant HHVM_METHOD(mysqli, dump_debug_info) {
  auto conn = get_connection(this_);
  VALIDATE_CONN_CONNECTED(conn);
  return !mysql_dump_debug_info(conn->get());
}

static Variant HHVM_METHOD(mysqli, get_charset) {
  MY_CHARSET_INFO cs;
  auto conn = get_connection(this_);
  VALIDATE_CONN_CONNECTED(conn);
  mysql_get_character_set_info(conn->get(), &cs);

  Object ret(SystemLib::AllocStdClassObject());
  ret.o_set(s_charset, String(cs.csname, CopyString));
  ret.o_set(s_collation, String(cs.name, CopyString));
  ret.o_set(s_dir, String(cs.dir, CopyString));
  ret.o_set(s_min_length, (int64_t)cs.mbminlen);
  ret.o_set(s_max_length, (int64_t)cs.mbmaxlen);
  ret.o_set(s_number, (int64_t)cs.number);
  ret.o_set(s_state, (int64_t)cs.state);
  ret.o_set(s_comment, String(cs.comment, CopyString));
  return ret;
}

//static Variant HHVM_METHOD(mysqli, get_connection_stats) {
//  throw NotImplementedException(__FUNCTION__);
//}

static Variant HHVM_METHOD(mysqli, hh_field_count) {
  auto conn = get_connection(this_);
  VALIDATE_CONN_CONNECTED(conn);
  return (int64_t)mysql_field_count(conn->get());
}

static Variant HHVM_METHOD(mysqli, hh_get_connection, int64_t state) {
  auto res = get_connection_resource(this_);
  VALIDATE_RESOURCE(res, state)
  return res;
}

static Variant HHVM_METHOD(mysqli, hh_get_result, bool use_store) {
  auto res = get_connection_resource(this_);
  VALIDATE_RESOURCE(res, MySQLState::CONNECTED)
  return php_mysql_get_result(res, use_store);
}

static void HHVM_METHOD(mysqli, hh_init) {
  Resource data = new MySQL(nullptr, 0, nullptr, nullptr, nullptr);
  this_->o_set(s_connection, data, s_mysqli.get());
}

static bool HHVM_METHOD(mysqli, hh_real_connect, const Variant& server,
                        const Variant& username, const Variant& password,
                        const Variant& dbname, const Variant& client_flags) {
  bool persistent = false;
  String s = server.toString();
  if (s.substr(0, 2).equal(s_persistent_prefix)) {
    persistent = true;
    s = s.substr(2);
  }
  auto conn = get_connection(this_);
  assert(conn);
  Variant ret = php_mysql_do_connect_on_link(
                  conn, s, username.toString(), password.toString(),
                  dbname.toString(), client_flags.toInt64(), persistent, false,
                  -1, -1);
  return ret.toBoolean();
}

static Variant HHVM_METHOD(mysqli, hh_real_query, const String& query) {
  auto res = get_connection_resource(this_);
  VALIDATE_RESOURCE(res, MySQLState::CONNECTED)
  return php_mysql_do_query(query, res, false);
}

static Variant HHVM_METHOD(mysqli, hh_server_version) {
  auto conn = get_connection(this_);
  VALIDATE_CONN_CONNECTED(conn);
  return (int64_t)mysql_get_server_version(conn->get());
}

static Variant HHVM_METHOD(mysqli, hh_sqlstate) {
  auto conn = get_connection(this_);
  VALIDATE_CONN_CONNECTED(conn);
  return String(mysql_sqlstate(conn->get()), CopyString);
}

static void HHVM_METHOD(mysqli, hh_update_last_error, Object stmt_obj) {
  auto conn = get_connection(this_);
  assert(conn);

  auto stmt = getStmt(stmt_obj);
  assert(stmt);

  auto s = stmt->get();
  auto mysql = conn->get();

  auto last_errno = mysql_stmt_errno(s);
  char last_error[MYSQL_ERRMSG_SIZE];
  char sqlstate[SQLSTATE_LENGTH + 1];
  memcpy(last_error, mysql_stmt_error(s), MYSQL_ERRMSG_SIZE);
  memcpy(sqlstate, mysql->net.sqlstate, SQLSTATE_LENGTH + 1);

  // This will clear the error both on the stmt and connection so we make sure
  // it closed now. Otherwise it will happen when the object is swept later
  stmt->close();

  // The MySQL C API documentation say that you shouldn't touch the internals of
  // the MySQL's datastructurs. But this is how Zend does it and there is no
  // other good way
  mysql->net.last_errno = last_errno;
  memcpy(mysql->net.last_error, last_error, MYSQL_ERRMSG_SIZE);
  memcpy(mysql->net.sqlstate, sqlstate, SQLSTATE_LENGTH + 1);
}

static Variant HHVM_METHOD(mysqli, kill, int64_t processid) {
  auto conn = get_connection(this_);
  VALIDATE_CONN_CONNECTED(conn);
  return !mysql_kill(conn->get(), processid);
}

static DataType get_option_value_type(int64_t option) {
  switch (option) {
    case MYSQL_INIT_COMMAND:
    case MYSQL_READ_DEFAULT_FILE:
    case MYSQL_READ_DEFAULT_GROUP:
    case MYSQL_SET_CHARSET_DIR:
    case MYSQL_SET_CHARSET_NAME:
#if MYSQL_VERSION_ID >= 50606
    case MYSQL_SERVER_PUBLIC_KEY:
#endif
#if MYSQL_VERSION_ID >= 40101
    case MYSQL_SET_CLIENT_IP:
#endif
#if MYSQL_VERSION_ID >= 40100
    case MYSQL_SHARED_MEMORY_BASE_NAME:
#endif
      return KindOfString;
    case MYSQL_OPT_CONNECT_TIMEOUT:
    case MYSQL_OPT_LOCAL_INFILE:
#if MYSQL_VERSION_ID >= 40101
    case MYSQL_OPT_READ_TIMEOUT:
    case MYSQL_OPT_WRITE_TIMEOUT:
#endif
#if MYSQL_VERSION_ID >= 40100
    case MYSQL_OPT_PROTOCOL:
#endif
      return KindOfInt64;
#if MYSQL_VERSION_ID >= 50610
    case MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS:
#endif
#if MYSQL_VERSION_ID >= 50023
    case MYSQL_OPT_SSL_VERIFY_SERVER_CERT:
#endif
#if MYSQL_VERSION_ID >= 50013
    case MYSQL_OPT_RECONNECT:
#endif
#if MYSQL_VERSION_ID >= 50003
    case MYSQL_REPORT_DATA_TRUNCATION:
#endif
#if MYSQL_VERSION_ID >= 40101
    case MYSQL_SECURE_AUTH:
#endif
      return KindOfBoolean;
    case MYSQL_OPT_COMPRESS:
    case MYSQL_OPT_NAMED_PIPE:
#if MYSQL_VERSION_ID >= 40101
    case MYSQL_OPT_GUESS_CONNECTION:
    case MYSQL_OPT_USE_EMBEDDED_CONNECTION:
    case MYSQL_OPT_USE_REMOTE_CONNECTION:
#endif
      return KindOfNull;
    default:
      return KindOfUnknown;
  }

  not_reached();
}

static Variant HHVM_METHOD(mysqli, options, int64_t option, const Variant& value) {
  auto conn = get_connection(this_);
  VALIDATE_CONN(conn, MySQLState::INITED)

  DataType dt = get_option_value_type(option);
  if (dt == KindOfUnknown) {
    return false;
  }

  // Just holders for the value
  my_bool bool_value;
  Variant other_value;

  const void *value_ptr = nullptr;
  if (!value.isNull()) {
    switch (dt) {
      case KindOfString:
        other_value = value.toString();
        value_ptr = other_value.getStringData()->data();
        break;
      case KindOfInt64:
        other_value = value.toInt64();
        value_ptr = other_value.getInt64Data();
        break;
      case KindOfBoolean:
        bool_value = value.toBoolean();
        value_ptr = &bool_value;
        break;
      case KindOfNull:
        break;
      default:
        not_reached();
    }
  }

  return !mysql_options(conn->get(), (mysql_option)option, value_ptr);
}

//static int64_t HHVM_STATIC_METHOD(mysqli, poll, VRefParam read,
//                                  VRefParam error, VRefParam reject,
//                                  int64_t sec, int64_t usec) {
//  throw NotImplementedException(__FUNCTION__);
//}
//
//static Object HHVM_METHOD(mysqli, reap_async_query) {
//  throw NotImplementedException(__FUNCTION__);
//}

static Variant HHVM_METHOD(mysqli, refresh, int64_t options) {
  auto conn = get_connection(this_);
  VALIDATE_CONN_CONNECTED(conn);
  return !mysql_refresh(conn->get(), options);
}

//static int64_t HHVM_METHOD(mysqli, rpl_query_type, const String& query) {
//  throw NotImplementedException(__FUNCTION__);
//}
//
//static bool HHVM_METHOD(mysqli, send_query, const String& query) {
//  throw NotImplementedException(__FUNCTION__);
//}
//
//static bool HHVM_METHOD(mysqli, set_local_infile_handler, const Object& read_func) {
//  throw NotImplementedException(__FUNCTION__);
//}

static Variant HHVM_METHOD(mysqli, ssl_set, const Variant& key,
                           const Variant& cert, const Variant& ca,
                           const Variant& capath, const Variant& cipher) {
  auto get_str_ptr = [](const Variant& v) -> const char* {
    return v.isNull() ? nullptr : v.toString().c_str();
  };
  auto conn = get_connection(this_);
  VALIDATE_CONN(conn, MySQLState::INITED);
  mysql_ssl_set(conn->get(), get_str_ptr(key), get_str_ptr(cert),
                get_str_ptr(ca), get_str_ptr(capath), get_str_ptr(cipher));
  return true;
}

#undef VALIDATE_CONN
#undef VALIDATE_CONN_CONNECTED

//////////////////////////////////////////////////////////////////////////////
// class mysqli_driver

//static void HHVM_METHOD(mysqli_driver, embedded_server_end) {
//  throw NotImplementedException(__FUNCTION__);
//}
//
//static bool HHVM_METHOD(mysqli_driver, embedded_server_start, bool start,
//                        const Array& arguments, const Array& groups) {
//  throw NotImplementedException(__FUNCTION__);
//}

//////////////////////////////////////////////////////////////////////////////
// class mysqli_result

#define VALIDATE_RESULT(res)                                                   \
  if (!res || !res->get()) {                                                   \
    raise_warning("invalid object or resource mysqli_result");                 \
    return init_null();                                                        \
  }

static Variant HHVM_METHOD(mysqli_result, hh_field_tell) {
  auto res = getResult(this_);
  VALIDATE_RESULT(res)
  return res->tellField();
}

static Variant HHVM_METHOD(mysqli_result, fetch_field) {
  auto res = getResult(this_);
  VALIDATE_RESULT(res)

  auto info = res->fetchFieldInfo();
  if (!info) {
    return false;
  }

  Object obj(SystemLib::AllocStdClassObject());
  obj->o_set("name",       info->name);
  obj->o_set("orgname",    info->org_name);
  obj->o_set("table",      info->table);
  obj->o_set("orgtable",   info->org_table);
  obj->o_set("def",        info->def);
  obj->o_set("db",         info->db);
  obj->o_set("catalog",    s_def);
  obj->o_set("max_length", info->max_length);
  obj->o_set("length",     info->length);
  obj->o_set("charsetnr",  (int64_t)info->charsetnr);
  obj->o_set("flags",      (int64_t)info->flags);
  obj->o_set("type",       info->type);
  obj->o_set("decimals",   (int64_t)info->decimals);

  return obj;
}

#undef VALIDATE_RESULT

//////////////////////////////////////////////////////////////////////////////
// class mysqli_stmt

static Variant HHVM_METHOD(mysqli_stmt, attr_get, int64_t attr) {
  return getStmt(this_)->attr_get(attr);
}

static Variant HHVM_METHOD(mysqli_stmt, attr_set, int64_t attr, int64_t mode) {
  return getStmt(this_)->attr_set(attr, mode);
}

static TypedValue* HHVM_MN(mysqli_stmt, bind_param)(ActRec* ar) {
  return bind_param_helper(ar->getThis(), ar, 0);
}

static TypedValue* HHVM_MN(mysqli_stmt, bind_result)(ActRec* ar) {
  return bind_result_helper(ar->getThis(), ar, 0);
}

static Variant HHVM_METHOD(mysqli_stmt, close) {
  return getStmt(this_)->close();
}

static void HHVM_METHOD(mysqli_stmt, data_seek, int64_t offset) {
  getStmt(this_)->data_seek(offset);
}

static Variant HHVM_METHOD(mysqli_stmt, execute) {
  return getStmt(this_)->execute();
}

static Variant HHVM_METHOD(mysqli_stmt, fetch) {
  return getStmt(this_)->fetch();
}

static void HHVM_METHOD(mysqli_stmt, free_result) {
  getStmt(this_)->free_result();
}

//static Object HHVM_METHOD(mysqli_stmt, get_result) {
//  throw NotImplementedException("mysqli_stmt::get_result");
//}

static Variant HHVM_METHOD(mysqli_stmt, hh_affected_rows) {
  return getStmt(this_)->affected_rows();
}

static Variant HHVM_METHOD(mysqli_stmt, hh_errno) {
  return getStmt(this_)->get_errno();
}

static Variant HHVM_METHOD(mysqli_stmt, hh_error) {
  return getStmt(this_)->get_error();
}

static Variant HHVM_METHOD(mysqli_stmt, hh_field_count) {
  return getStmt(this_)->field_count();
}

static void HHVM_METHOD(mysqli_stmt, hh_init, Variant connection) {
  Object obj = connection.toObject();
  auto data = NEWOBJ(MySQLStmt)(get_connection(obj)->get());
  this_->o_set(s_stmt, Resource(data), s_mysqli_stmt.get());
}

static Variant HHVM_METHOD(mysqli_stmt, hh_insert_id) {
  return getStmt(this_)->insert_id();
}

static Variant HHVM_METHOD(mysqli_stmt, hh_num_rows) {
  return getStmt(this_)->num_rows();
}

static Variant HHVM_METHOD(mysqli_stmt, hh_param_count) {
  return getStmt(this_)->param_count();
}

static Variant HHVM_METHOD(mysqli_stmt, prepare, const String& query) {
  return getStmt(this_)->prepare(query);
}

static Variant HHVM_METHOD(mysqli_stmt, reset) {
  return getStmt(this_)->reset();
}

static Variant HHVM_METHOD(mysqli_stmt, result_metadata) {
  return getStmt(this_)->result_metadata();
}

static Variant HHVM_METHOD(mysqli_stmt, send_long_data, int64_t param_nr,
                           const String& data) {
  return getStmt(this_)->send_long_data(param_nr, data);
}

static Variant HHVM_METHOD(mysqli_stmt, store_result) {
  return getStmt(this_)->store_result();
}

//////////////////////////////////////////////////////////////////////////////
// class mysqli_warning

//static void HHVM_METHOD(mysqli_warning, __construct) {
//  throw NotImplementedException(__FUNCTION__);
//}
//
//static void HHVM_METHOD(mysqli_warning, next) {
//  throw NotImplementedException(__FUNCTION__);
//}

//////////////////////////////////////////////////////////////////////////////
// functions

static int64_t HHVM_FUNCTION(mysqli_get_client_version) {
  return mysql_get_client_version();
}

//static Array HHVM_FUNCTION(mysqli_get_client_stats) {
//  throw NotImplementedException(__FUNCTION__);
//}
//
//static void HHVM_FUNCTION(mysqli_set_local_infile_default, const Object& link) {
//  throw NotImplementedException(__FUNCTION__);
//}

static TypedValue* HHVM_FN(mysqli_stmt_bind_param)(ActRec* ar) {
  return bind_param_helper(getArg<KindOfObject>(ar, 0), ar, 1);
}

static TypedValue* HHVM_FN(mysqli_stmt_bind_result)(ActRec* ar) {
  return bind_result_helper(getArg<KindOfObject>(ar, 0), ar, 1);
}

static bool HHVM_FUNCTION(mysqli_thread_safe) {
  return mysql_thread_safe();
}

//////////////////////////////////////////////////////////////////////////////

class mysqliExtension : public Extension {
 public:
  mysqliExtension() : Extension("mysqli") {}
  virtual void moduleInit() {
    // mysqli
    HHVM_ME(mysqli, autocommit);
    HHVM_ME(mysqli, change_user);
    HHVM_ME(mysqli, character_set_name);
    HHVM_ME(mysqli, dump_debug_info);
    HHVM_ME(mysqli, get_charset);
    //HHVM_ME(mysqli, get_connection_stats); // MYSQLND
    HHVM_ME(mysqli, hh_field_count);
    HHVM_ME(mysqli, hh_get_connection);
    HHVM_ME(mysqli, hh_get_result);
    HHVM_ME(mysqli, hh_init);
    HHVM_ME(mysqli, hh_real_connect);
    HHVM_ME(mysqli, hh_real_query);
    HHVM_ME(mysqli, hh_server_version);
    HHVM_ME(mysqli, hh_sqlstate);
    HHVM_ME(mysqli, hh_update_last_error);
    HHVM_ME(mysqli, kill);
    HHVM_ME(mysqli, options);
    //HHVM_STATIC_ME(mysqli, poll); // MYSQLND
    //HHVM_ME(mysqli, reap_async_query); // MYSQLND
    HHVM_ME(mysqli, refresh);
    //HHVM_ME(mysqli, set_local_infile_handler);
    HHVM_ME(mysqli, ssl_set);

    // mysqli_result
    HHVM_ME(mysqli_result, hh_field_tell);
    HHVM_ME(mysqli_result, fetch_field);

    // mysqli_stmt
    HHVM_ME(mysqli_stmt, attr_get);
    HHVM_ME(mysqli_stmt, attr_set);
    HHVM_ME(mysqli_stmt, bind_param);
    HHVM_ME(mysqli_stmt, bind_result);
    HHVM_ME(mysqli_stmt, close);
    HHVM_ME(mysqli_stmt, data_seek);
    HHVM_ME(mysqli_stmt, execute);
    HHVM_ME(mysqli_stmt, fetch);
    HHVM_ME(mysqli_stmt, free_result);
    //HHVM_ME(mysqli_stmt, get_result); // MYSQLND
    HHVM_ME(mysqli_stmt, hh_affected_rows);
    HHVM_ME(mysqli_stmt, hh_errno);
    HHVM_ME(mysqli_stmt, hh_error);
    HHVM_ME(mysqli_stmt, hh_field_count);
    HHVM_ME(mysqli_stmt, hh_init);
    HHVM_ME(mysqli_stmt, hh_insert_id);
    HHVM_ME(mysqli_stmt, hh_num_rows);
    HHVM_ME(mysqli_stmt, hh_param_count);
    HHVM_ME(mysqli_stmt, prepare);
    HHVM_ME(mysqli_stmt, reset);
    HHVM_ME(mysqli_stmt, result_metadata);
    HHVM_ME(mysqli_stmt, send_long_data);
    HHVM_ME(mysqli_stmt, store_result);

    HHVM_FE(mysqli_get_client_version);
    //HHVM_FE(mysqli_get_client_stats);
    //HHVM_FE(mysqli_set_local_infile_default);
    HHVM_FE(mysqli_stmt_bind_param);
    HHVM_FE(mysqli_stmt_bind_result);
    HHVM_FE(mysqli_thread_safe);

#define REGISTER_CONST_VALUE(option, value)                                    \
  Native::registerConstant<KindOfInt64>(makeStaticString("MYSQLI_" #option),   \
                                        (value));
#define REGISTER_CONST(option)                                                 \
  Native::registerConstant<KindOfInt64>(makeStaticString("MYSQLI_" #option),   \
                                        (option));
#define REGISTER_CONST_MIRROR(option)                                          \
  Native::registerConstant<KindOfInt64>(makeStaticString("MYSQLI_" #option),   \
                                        (MYSQL_##option));

    // Fetch type
    REGISTER_CONST_VALUE(ASSOC, 1)
    REGISTER_CONST_VALUE(NUM, 2)
    REGISTER_CONST_VALUE(BOTH, 3)

    // Fetch return
    REGISTER_CONST_MIRROR(NO_DATA)
    REGISTER_CONST_MIRROR(DATA_TRUNCATED)

    // Result type
    REGISTER_CONST_VALUE(STORE_RESULT, 0)
    REGISTER_CONST_VALUE(USE_RESULT, 1)

    // Report options
    REGISTER_CONST_VALUE(REPORT_OFF, 0)
    REGISTER_CONST_VALUE(REPORT_ERROR, 1)
    REGISTER_CONST_VALUE(REPORT_STRICT, 2)
    REGISTER_CONST_VALUE(REPORT_INDEX, 4)
    REGISTER_CONST_VALUE(REPORT_CLOSE, 8)
    REGISTER_CONST_VALUE(REPORT_ALL, 255)

    // Stmt Attr types
    REGISTER_CONST(STMT_ATTR_UPDATE_MAX_LENGTH)
    REGISTER_CONST(STMT_ATTR_CURSOR_TYPE)
    REGISTER_CONST(STMT_ATTR_PREFETCH_ROWS)

    // Stmt Attr Cursor type
    REGISTER_CONST(CURSOR_TYPE_NO_CURSOR)
    REGISTER_CONST(CURSOR_TYPE_READ_ONLY)
    REGISTER_CONST(CURSOR_TYPE_FOR_UPDATE)
    REGISTER_CONST(CURSOR_TYPE_SCROLLABLE)

    // Field flags
    REGISTER_CONST(NOT_NULL_FLAG)
    REGISTER_CONST(PRI_KEY_FLAG)
    REGISTER_CONST(UNIQUE_KEY_FLAG)
    REGISTER_CONST(MULTIPLE_KEY_FLAG)
    REGISTER_CONST(UNSIGNED_FLAG)
    REGISTER_CONST(ZEROFILL_FLAG)
    REGISTER_CONST(BINARY_FLAG)
    REGISTER_CONST(AUTO_INCREMENT_FLAG)
    REGISTER_CONST(ENUM_FLAG)
    REGISTER_CONST(SET_FLAG)
    REGISTER_CONST(BLOB_FLAG)
    REGISTER_CONST(TIMESTAMP_FLAG)
    REGISTER_CONST(NUM_FLAG)
    REGISTER_CONST(PART_KEY_FLAG)
    REGISTER_CONST(GROUP_FLAG)
#if MYSQL_VERSION_ID > 50001
    REGISTER_CONST(NO_DEFAULT_VALUE_FLAG)
#endif

    // Refresh options
    REGISTER_CONST(REFRESH_GRANT)
    REGISTER_CONST(REFRESH_LOG)
    REGISTER_CONST(REFRESH_TABLES)
    REGISTER_CONST(REFRESH_HOSTS)
    REGISTER_CONST(REFRESH_STATUS)
    REGISTER_CONST(REFRESH_THREADS)
    REGISTER_CONST(REFRESH_SLAVE)
    REGISTER_CONST(REFRESH_MASTER)

    // Transaction flags
    REGISTER_CONST_VALUE(TRANS_START_WITH_CONSISTENT_SNAPSHOT, 1)
    REGISTER_CONST_VALUE(TRANS_START_READ_WRITE, 2)
    REGISTER_CONST_VALUE(TRANS_START_READ_ONLY, 4)
    REGISTER_CONST_VALUE(TRANS_COR_AND_CHAIN, 1)
    REGISTER_CONST_VALUE(TRANS_COR_AND_NO_CHAIN, 2)
    REGISTER_CONST_VALUE(TRANS_COR_RELEASE, 4)
    REGISTER_CONST_VALUE(TRANS_COR_NO_RELEASE, 8)

    // Connection flags
    REGISTER_CONST(CLIENT_SSL)
    REGISTER_CONST(CLIENT_COMPRESS)
    REGISTER_CONST(CLIENT_INTERACTIVE)
    REGISTER_CONST(CLIENT_IGNORE_SPACE)
    REGISTER_CONST(CLIENT_NO_SCHEMA)
    REGISTER_CONST(CLIENT_FOUND_ROWS)

    // Field types
    REGISTER_CONST_MIRROR(TYPE_TINY)
    REGISTER_CONST_MIRROR(TYPE_SHORT)
    REGISTER_CONST_MIRROR(TYPE_INT24)
    REGISTER_CONST_MIRROR(TYPE_LONG)
    REGISTER_CONST_MIRROR(TYPE_LONGLONG)
    REGISTER_CONST_MIRROR(TYPE_DECIMAL)
    REGISTER_CONST_MIRROR(TYPE_FLOAT)
    REGISTER_CONST_MIRROR(TYPE_DOUBLE)
    REGISTER_CONST_MIRROR(TYPE_TIMESTAMP)
    REGISTER_CONST_MIRROR(TYPE_DATE)
    REGISTER_CONST_MIRROR(TYPE_NEWDATE)
    REGISTER_CONST_MIRROR(TYPE_TIME)
    REGISTER_CONST_MIRROR(TYPE_DATETIME)
    REGISTER_CONST_MIRROR(TYPE_YEAR)
    REGISTER_CONST_MIRROR(TYPE_ENUM)
    REGISTER_CONST_MIRROR(TYPE_SET)
    REGISTER_CONST_MIRROR(TYPE_TINY_BLOB)
    REGISTER_CONST_MIRROR(TYPE_MEDIUM_BLOB)
    REGISTER_CONST_MIRROR(TYPE_LONG_BLOB)
    REGISTER_CONST_MIRROR(TYPE_BLOB)
    REGISTER_CONST_MIRROR(TYPE_VARCHAR)
    REGISTER_CONST_MIRROR(TYPE_VAR_STRING)
    REGISTER_CONST_MIRROR(TYPE_STRING)
    REGISTER_CONST_MIRROR(TYPE_GEOMETRY)
    REGISTER_CONST_MIRROR(TYPE_NULL)
#if MYSQL_VERSION_ID > 50002
    REGISTER_CONST_MIRROR(TYPE_NEWDECIMAL)
    REGISTER_CONST_MIRROR(TYPE_BIT)
#endif

    // Zend have some extra that is just maps to some other type
    REGISTER_CONST_VALUE(TYPE_INTERVAL, MYSQL_TYPE_ENUM)
    REGISTER_CONST_VALUE(TYPE_CHAR, MYSQL_TYPE_TINY)

    // Options
    REGISTER_CONST_MIRROR(INIT_COMMAND)
    REGISTER_CONST_MIRROR(OPT_COMPRESS)
    REGISTER_CONST_MIRROR(OPT_CONNECT_TIMEOUT)
    REGISTER_CONST_MIRROR(OPT_LOCAL_INFILE)
    REGISTER_CONST_MIRROR(OPT_NAMED_PIPE)
    REGISTER_CONST_MIRROR(READ_DEFAULT_FILE)
    REGISTER_CONST_MIRROR(READ_DEFAULT_GROUP)
    REGISTER_CONST_MIRROR(SET_CHARSET_DIR)
    REGISTER_CONST_MIRROR(SET_CHARSET_NAME)
#if MYSQL_VERSION_ID >= 50610
    REGISTER_CONST_MIRROR(OPT_CAN_HANDLE_EXPIRED_PASSWORDS)
#endif
#if MYSQL_VERSION_ID >= 50606
    REGISTER_CONST_MIRROR(SERVER_PUBLIC_KEY)
#endif
#if MYSQL_VERSION_ID >= 50023
    REGISTER_CONST_MIRROR(OPT_SSL_VERIFY_SERVER_CERT)
#endif
#if MYSQL_VERSION_ID >= 50013
    REGISTER_CONST_MIRROR(OPT_RECONNECT)
#endif
#if MYSQL_VERSION_ID >= 50003
    REGISTER_CONST_MIRROR(REPORT_DATA_TRUNCATION)
#endif
#if MYSQL_VERSION_ID >= 40101
    REGISTER_CONST_MIRROR(OPT_GUESS_CONNECTION)
    REGISTER_CONST_MIRROR(OPT_READ_TIMEOUT)
    REGISTER_CONST_MIRROR(OPT_USE_EMBEDDED_CONNECTION)
    REGISTER_CONST_MIRROR(OPT_USE_REMOTE_CONNECTION)
    REGISTER_CONST_MIRROR(OPT_WRITE_TIMEOUT)
    REGISTER_CONST_MIRROR(SECURE_AUTH)
    REGISTER_CONST_MIRROR(SET_CLIENT_IP)
#endif
#if MYSQL_VERSION_ID >= 40100
    REGISTER_CONST_MIRROR(OPT_PROTOCOL)
    REGISTER_CONST_MIRROR(SHARED_MEMORY_BASE_NAME)
#endif

    // Misc
    REGISTER_CONST_VALUE(DEBUG_TRACE_ENABLED, 0)
    REGISTER_CONST_VALUE(SERVER_QUERY_NO_GOOD_INDEX_USED, 16)
    REGISTER_CONST_VALUE(SERVER_QUERY_NO_INDEX_USED, 32)
    REGISTER_CONST_VALUE(SERVER_QUERY_WAS_SLOW, 2048)

#undef REGISTER_CONST_VALUE
#undef REGISTER_CONST
#undef REGISTER_CONST_MIRROR

    loadSystemlib();
  }
} s_mysqli_extension;

// Uncomment for non-bundled module
//HHVM_GET_MODULE(mysqli);

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
