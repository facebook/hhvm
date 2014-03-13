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

#include "hphp/runtime/ext/pdo_mysql.h"
#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "mysql.h"

#ifdef PHP_MYSQL_UNIX_SOCK_ADDR
#ifdef MYSQL_UNIX_ADDR
#undef MYSQL_UNIX_ADDR
#endif
#define MYSQL_UNIX_ADDR PHP_MYSQL_UNIX_SOCK_ADDR
#endif

namespace HPHP {

IMPLEMENT_DEFAULT_EXTENSION_VERSION(pdo_mysql, 1.0.2);

///////////////////////////////////////////////////////////////////////////////

class PDOMySqlError {
public:
  PDOMySqlError() : file(NULL), line(0), errcode(0), errmsg(NULL) {
  }

  const char *file;
  int line;
  unsigned int errcode;
  char *errmsg;
};

class PDOMySqlStatement;
class PDOMySqlConnection : public PDOConnection {
public:
  PDOMySqlConnection();
  virtual ~PDOMySqlConnection();
  virtual bool create(const Array& options);

  int handleError(const char *file, int line, PDOMySqlStatement *stmt = NULL);

  virtual bool support(SupportedMethod method);
  virtual bool closer();
  virtual bool preparer(const String& sql, sp_PDOStatement *stmt, const Variant& options);
  virtual int64_t doer(const String& sql);
  virtual bool quoter(const String& input, String &quoted, PDOParamType paramtype);
  virtual bool begin();
  virtual bool commit();
  virtual bool rollback();
  virtual bool setAttribute(int64_t attr, const Variant& value);
  virtual String lastId(const char *name);
  virtual bool fetchErr(PDOStatement *stmt, Array &info);
  virtual int getAttribute(int64_t attr, Variant &value);
  virtual bool checkLiveness();
  virtual void persistentShutdown();

  bool buffered() const { return m_buffered;}
  unsigned long max_buffer_size() const { return m_max_buffer_size;}
  bool fetch_table_names() const { return m_fetch_table_names;}

private:
  MYSQL         *m_server;
  unsigned       m_attached:1;
  unsigned       m_buffered:1;
  unsigned       m_emulate_prepare:1;
  unsigned       m_fetch_table_names:1;
  unsigned long  m_max_buffer_size;
  PDOMySqlError  m_einfo;
};

class PDOMySqlStatement : public PDOStatement {
public:
  DECLARE_RESOURCE_ALLOCATION(PDOMySqlStatement);
  PDOMySqlStatement(PDOMySqlConnection *conn, MYSQL *server);
  virtual ~PDOMySqlStatement();

  bool create(const String& sql, const Array& options);

  virtual bool support(SupportedMethod method);
  virtual bool executer();
  virtual bool fetcher(PDOFetchOrientation ori, long offset);
  virtual bool describer(int colno);
  virtual bool getColumn(int colno, Variant &value);
  virtual bool paramHook(PDOBoundParam *param, PDOParamEvent event_type);
  virtual bool getColumnMeta(int64_t colno, Array &return_value);
  virtual bool nextRowset();
  virtual bool cursorCloser();

  MYSQL_STMT *stmt() { return m_stmt;}

private:
  PDOMySqlConnection *m_conn;
  MYSQL              *m_server;
  MYSQL_RES          *m_result;
  const MYSQL_FIELD  *m_fields;
  MYSQL_ROW           m_current_data;
  long               *m_current_lengths;
  PDOMySqlError       m_einfo;
  MYSQL_STMT         *m_stmt;
  int                 m_num_params;
  MYSQL_BIND         *m_params;
  my_bool            *m_in_null;
  unsigned long      *m_in_length;
  MYSQL_BIND         *m_bound_result;
  my_bool            *m_out_null;
  unsigned long      *m_out_length;
  unsigned int        m_params_given;
  unsigned            m_max_length:1;

  void setRowCount();
  bool executePrepared();
  int handleError(const char *file, int line);
};

///////////////////////////////////////////////////////////////////////////////

/* For the convenience of drivers, this function will parse a data source
 * string, of the form "name=value; name2=value2" and populate variables
 * according to the data you pass in and array of pdo_data_src_parser structures */
struct pdo_data_src_parser {
  const char *optname;
  char *optval;
  int freeme;
};

static int php_pdo_parse_data_source(const char *data_source,
                                     int data_source_len,
                                     struct pdo_data_src_parser *parsed,
                                     int nparams) {
  int i, j;
  int valstart = -1;
  int semi = -1;
  int optstart = 0;
  int nlen;
  int n_matches = 0;

  i = 0;
  while (i < data_source_len) {
    /* looking for NAME= */

    if (data_source[i] == '\0') {
      break;
    }

    if (data_source[i] != '=') {
      ++i;
      continue;
    }

    valstart = ++i;

    /* now we're looking for VALUE; or just VALUE<NUL> */
    semi = -1;
    while (i < data_source_len) {
      if (data_source[i] == '\0') {
        semi = i++;
        break;
      }
      if (data_source[i] == ';') {
        semi = i++;
        break;
      }
      ++i;
    }

    if (semi == -1) {
      semi = i;
    }

    /* find the entry in the array */
    nlen = valstart - optstart - 1;
    for (j = 0; j < nparams; j++) {
      if (0 == strncmp(data_source + optstart, parsed[j].optname, nlen) &&
          parsed[j].optname[nlen] == '\0') {
        /* got a match */
        if (parsed[j].freeme) {
          free(parsed[j].optval);
        }
        parsed[j].optval = strndup(data_source + valstart, semi - valstart);
        parsed[j].freeme = 1;
        ++n_matches;
        break;
      }
    }

    while (i < data_source_len && isspace(data_source[i])) {
      i++;
    }

    optstart = i;
  }

  return n_matches;
}

static long pdo_attr_lval(const Array& options, int opt, long defaultValue) {
  if (options.exists(opt)) {
    return options[opt].toInt64();
  }
  return defaultValue;
}

static String pdo_attr_strval(const Array& options, int opt, const char *def) {
  if (options.exists(opt)) {
    return options[opt].toString();
  }
  if (def) {
    return def;
  }
  return String();
}

///////////////////////////////////////////////////////////////////////////////

PDOMySqlConnection::PDOMySqlConnection()
    : m_server(NULL), m_attached(0), m_buffered(0), m_emulate_prepare(0),
      m_fetch_table_names(0), m_max_buffer_size(0) {
}

PDOMySqlConnection::~PDOMySqlConnection() {
  if (m_server) {
    mysql_close(m_server);
  }
  if (m_einfo.errmsg) {
    free(m_einfo.errmsg);
  }
}

bool PDOMySqlConnection::create(const Array& options) {
  int i, ret = 0;
  char *host = NULL, *unix_socket = NULL;
  unsigned int port = 3306;
  char *dbname;
  char *charset = nullptr;
  struct pdo_data_src_parser vars[] = {
    { "charset",      nullptr,          0 },
    { "dbname",       "",               0 },
    { "host",         "localhost",      0 },
    { "port",         "3306",           0 },
    { "unix_socket",  MYSQL_UNIX_ADDR,  0 },
  };
  int connect_opts = 0
#ifdef CLIENT_MULTI_RESULTS
    |CLIENT_MULTI_RESULTS
#endif
#ifdef CLIENT_MULTI_STATEMENTS
    |CLIENT_MULTI_STATEMENTS
#endif
    ;

  php_pdo_parse_data_source(data_source.data(), data_source.size(), vars, 5);

  /* handle for the server */
  if (!(m_server = mysql_init(NULL))) {
    handleError(__FILE__, __LINE__);
    goto cleanup;
  }

  m_max_buffer_size = 1024*1024;
  m_buffered = m_emulate_prepare = 1;
  charset = vars[0].optval;

  /* handle MySQL options */
  if (!options.empty()) {
    long connect_timeout = pdo_attr_lval(options, PDO_ATTR_TIMEOUT, 30);
    long local_infile = pdo_attr_lval(options, PDO_MYSQL_ATTR_LOCAL_INFILE, 0);
    String init_cmd, default_file, default_group;
    long compress = 0;
    m_buffered = pdo_attr_lval(options, PDO_MYSQL_ATTR_USE_BUFFERED_QUERY, 1);

    m_emulate_prepare = pdo_attr_lval(options, PDO_MYSQL_ATTR_DIRECT_QUERY,
                                      m_emulate_prepare);
    m_emulate_prepare = pdo_attr_lval(options, PDO_ATTR_EMULATE_PREPARES,
                                      m_emulate_prepare);

    m_max_buffer_size = pdo_attr_lval(options, PDO_MYSQL_ATTR_MAX_BUFFER_SIZE,
                                      m_max_buffer_size);

    if (pdo_attr_lval(options, PDO_MYSQL_ATTR_FOUND_ROWS, 0)) {
      connect_opts |= CLIENT_FOUND_ROWS;
    }
    if (pdo_attr_lval(options, PDO_MYSQL_ATTR_IGNORE_SPACE, 0)) {
      connect_opts |= CLIENT_IGNORE_SPACE;
    }

    if (mysql_options(m_server, MYSQL_OPT_CONNECT_TIMEOUT,
                      (const char *)&connect_timeout)) {
      handleError(__FILE__, __LINE__);
      goto cleanup;
    }

    if (mysql_options(m_server, MYSQL_OPT_LOCAL_INFILE,
                      (const char *)&local_infile)) {
      handleError(__FILE__, __LINE__);
      goto cleanup;
    }
#ifdef MYSQL_OPT_RECONNECT
    /* since 5.0.3, the default for this option is 0 if not specified.
     * we want the old behaviour */
    {
      long reconnect = 1;
      mysql_options(m_server, MYSQL_OPT_RECONNECT, (const char*)&reconnect);
    }
#endif
    init_cmd = pdo_attr_strval(options, PDO_MYSQL_ATTR_INIT_COMMAND, NULL);
    if (!init_cmd.empty()) {
      if (mysql_options(m_server, MYSQL_INIT_COMMAND, init_cmd.data())) {
        handleError(__FILE__, __LINE__);
        goto cleanup;
      }
    }

    default_file = pdo_attr_strval(options, PDO_MYSQL_ATTR_READ_DEFAULT_FILE,
                                   NULL);
    if (!default_file.empty()) {
      if (mysql_options(m_server, MYSQL_READ_DEFAULT_FILE,
                        default_file.data())) {
        handleError(__FILE__, __LINE__);
        goto cleanup;
      }
    }

    default_group = pdo_attr_strval(options, PDO_MYSQL_ATTR_READ_DEFAULT_GROUP,
                                    NULL);
    if (!default_group.empty()) {
      if (mysql_options(m_server, MYSQL_READ_DEFAULT_GROUP,
                        default_group.data())) {
        handleError(__FILE__, __LINE__);
        goto cleanup;
      }
    }

    compress = pdo_attr_lval(options, PDO_MYSQL_ATTR_COMPRESS, 0);
    if (compress) {
      if (mysql_options(m_server, MYSQL_OPT_COMPRESS, 0)) {
        handleError(__FILE__, __LINE__);
        goto cleanup;
      }
    }
  }

  if (charset) {
    if (mysql_options(m_server, MYSQL_SET_CHARSET_NAME, charset)) {
      handleError(__FILE__, __LINE__);
      goto cleanup;
    }
  }

  dbname = vars[1].optval;
  host = vars[2].optval;
  if (vars[3].optval) {
    port = atoi(vars[3].optval);
  }
  if (vars[2].optval && !strcmp("localhost", vars[2].optval)) {
    unix_socket = vars[4].optval;
  }

  /* TODO: - Check zval cache + ZTS */
  if (mysql_real_connect(m_server, host, username.c_str(), password.c_str(),
                         dbname, port, unix_socket, connect_opts) == NULL) {
    handleError(__FILE__, __LINE__);
    goto cleanup;
  }

  if (!auto_commit) {
    mysql_autocommit(m_server, auto_commit);
  }

  m_attached = 1;

  alloc_own_columns = 1;
  max_escaped_char_length = 2;

  ret = 1;

cleanup:
  for (i = 0; i < (int)(sizeof(vars)/sizeof(vars[0])); i++) {
    if (vars[i].freeme) {
      free(vars[i].optval);
    }
  }

  return ret;
}

bool PDOMySqlConnection::support(SupportedMethod method) {
  return true;
}

bool PDOMySqlConnection::closer() {
  if (m_server) {
    mysql_close(m_server);
    m_server = NULL;
  }
  if (m_einfo.errmsg) {
    free(m_einfo.errmsg);
    m_einfo.errmsg = NULL;
  }
  return false;
}

int PDOMySqlConnection::handleError(const char *file, int line,
                                    PDOMySqlStatement *stmt) {
  PDOErrorType *pdo_err;
  PDOMySqlError *einfo = &m_einfo;

  if (stmt) {
    pdo_err = &stmt->error_code;
  } else {
    pdo_err = &error_code;
  }

  if (stmt && stmt->stmt()) {
    einfo->errcode = mysql_stmt_errno(stmt->stmt());
  } else {
    einfo->errcode = mysql_errno(m_server);
  }

  einfo->file = file;
  einfo->line = line;

  if (einfo->errmsg) {
    free(einfo->errmsg);
    einfo->errmsg = NULL;
  }

  if (einfo->errcode) {
    if (einfo->errcode == 2014) {
      einfo->errmsg =
        strdup("Cannot execute queries while other unbuffered queries are "
               "active.  Consider using PDOStatement::fetchAll().  "
               "Alternatively, if your code is only ever going to run against "
               "mysql, you may enable query buffering by setting the "
               "PDO::MYSQL_ATTR_USE_BUFFERED_QUERY attribute.");
    } else if (einfo->errcode == 2057) {
      einfo->errmsg =
        strdup("A stored procedure returning result sets of different size "
               "was called. This is not supported by libmysql");
    } else {
      einfo->errmsg = strdup(mysql_error(m_server));
    }
  } else { /* no error */
    strcpy(*pdo_err, PDO_ERR_NONE);
    return false;
  }

  if (stmt && stmt->stmt()) {
    strcpy(*pdo_err, mysql_stmt_sqlstate(stmt->stmt()));
  } else {
    strcpy(*pdo_err, mysql_sqlstate(m_server));
  }

  if (stmt && stmt->stmt()) {
    pdo_raise_impl_error(stmt->dbh, NULL, pdo_err[0], einfo->errmsg);
  } else {
    throw_pdo_exception((int)einfo->errcode, uninit_null(), "SQLSTATE[%s] [%d] %s",
                        pdo_err[0], einfo->errcode, einfo->errmsg);
  }
  return einfo->errcode;
}

bool PDOMySqlConnection::preparer(const String& sql, sp_PDOStatement *stmt,
                                  const Variant& options) {
  PDOMySqlStatement *s = NEWOBJ(PDOMySqlStatement)(this, m_server);
  *stmt = s;

  if (m_emulate_prepare) {
    return true;
  }
  int server_version = mysql_get_server_version(m_server);
  if (server_version < 40100) {
    return true;
  }

  if (s->create(sql, options.toArray())) {
    alloc_own_columns = 1;
    return true;
  }

  stmt->reset();
  strcpy(error_code, s->error_code);
  return false;
}

int64_t PDOMySqlConnection::doer(const String& sql) {
  if (mysql_real_query(m_server, sql.data(), sql.size())) {
    handleError(__FILE__, __LINE__);
    return -1;
  }

  my_ulonglong c = mysql_affected_rows(m_server);
  if (c == (my_ulonglong) -1) {
    handleError(__FILE__, __LINE__);
    return m_einfo.errcode ? -1 : 0;
  }

  /* MULTI_QUERY support - eat up all unfetched result sets */
  while (mysql_more_results(m_server)) {
    if (mysql_next_result(m_server)) {
      return true;
    }
    MYSQL_RES *result = mysql_store_result(m_server);
    if (result) {
      mysql_free_result(result);
    }
  }
  return c;
}

bool PDOMySqlConnection::quoter(const String& input, String &quoted,
                                PDOParamType paramtype) {
  String s(2 * input.size() + 3, ReserveString);
  char *buf = s.bufferSlice().ptr;
  int len = mysql_real_escape_string(m_server, buf + 1,
                                     input.data(), input.size());
  len++;
  buf[0] = buf[len] = '\'';
  len++;
  quoted = s.setSize(len);
  return true;
}

bool PDOMySqlConnection::begin() {
  return doer("START TRANSACTION") >= 0;
}

bool PDOMySqlConnection::commit() {
  return mysql_commit(m_server) >= 0;
}

bool PDOMySqlConnection::rollback() {
  return mysql_rollback(m_server) >= 0;
}

bool PDOMySqlConnection::setAttribute(int64_t attr, const Variant& value) {
  switch (attr) {
  case PDO_ATTR_AUTOCOMMIT:
    /* ignore if the new value equals the old one */
    if (auto_commit ^ value.toBoolean()) {
      auto_commit = value.toBoolean();
      mysql_autocommit(m_server, auto_commit);
    }
    return true;

  case PDO_MYSQL_ATTR_USE_BUFFERED_QUERY:
    m_buffered = value.toBoolean();
    return true;
  case PDO_MYSQL_ATTR_DIRECT_QUERY:
  case PDO_ATTR_EMULATE_PREPARES:
    m_emulate_prepare = value.toBoolean();
    return true;
  case PDO_ATTR_FETCH_TABLE_NAMES:
    m_fetch_table_names = value.toBoolean();
    return true;
  case PDO_MYSQL_ATTR_MAX_BUFFER_SIZE:
    if (value.toInt64() < 0) {
      /* TODO: Johannes, can we throw a warning here? */
      m_max_buffer_size = 1024*1024;
    } else {
      m_max_buffer_size = value.toInt64();
    }
    return true;
  default:
    return false;
  }
}

String PDOMySqlConnection::lastId(const char *name) {
  return (int64_t)mysql_insert_id(m_server);
}

bool PDOMySqlConnection::fetchErr(PDOStatement *stmt, Array &info) {
  if (m_einfo.errcode) {
    info.append((int64_t)m_einfo.errcode);
    info.append(String(m_einfo.errmsg, CopyString));
  }
  return true;
}

int PDOMySqlConnection::getAttribute(int64_t attr, Variant &value) {
  switch (attr) {
  case PDO_ATTR_CLIENT_VERSION:
    value = String((char *)mysql_get_client_info(), CopyString);
    break;
  case PDO_ATTR_SERVER_VERSION:
    value = String((char *)mysql_get_server_info(m_server), CopyString);
    break;
  case PDO_ATTR_CONNECTION_STATUS:
    value = String((char *)mysql_get_host_info(m_server), CopyString);
    break;
  case PDO_ATTR_SERVER_INFO: {
    char *tmp = (char *)mysql_stat(m_server);
    if (tmp) {
      value = String(tmp, CopyString);
    } else {
      handleError(__FILE__, __LINE__);
      return -1;
    }
    break;
  }
  case PDO_ATTR_AUTOCOMMIT:
    value = (int64_t)auto_commit;
    break;
  case PDO_MYSQL_ATTR_USE_BUFFERED_QUERY:
    value = (int64_t)m_buffered;
    break;
  case PDO_MYSQL_ATTR_DIRECT_QUERY:
    value = (int64_t)m_emulate_prepare;
    break;
  case PDO_MYSQL_ATTR_MAX_BUFFER_SIZE:
    value = (int64_t)m_max_buffer_size;
    break;
  default:
    return 0;
  }
  return 1;
}

bool PDOMySqlConnection::checkLiveness() {
  return !mysql_ping(m_server);
}

void PDOMySqlConnection::persistentShutdown() {
  // do nothing
}

///////////////////////////////////////////////////////////////////////////////

void PDOMySqlStatement::setRowCount() {
  my_ulonglong count = mysql_stmt_affected_rows(m_stmt);
  if (count != (my_ulonglong)-1) {
    row_count = count;
  }
}

bool PDOMySqlStatement::executePrepared() {
  /* (re)bind the parameters */
  if (mysql_stmt_bind_param(m_stmt, m_params) || mysql_stmt_execute(m_stmt)) {
    if (m_params) {
      free(m_params);
      m_params = 0;
    }
    handleError(__FILE__, __LINE__);
    if (mysql_stmt_errno(m_stmt) == 2057) {
      /* CR_NEW_STMT_METADATA makes the statement unusable */
      m_stmt = NULL;
    }
    return false;
  }

  if (!m_result) {
    int i;

    /* figure out the result set format, if any */
    m_result = mysql_stmt_result_metadata(m_stmt);
    if (m_result) {
      int calc_max_length = m_conn->buffered() && m_max_length == 1;
      m_fields = mysql_fetch_fields(m_result);
      if (m_bound_result) {
        int i;
        for (i = 0; i < column_count; i++) {
          free(m_bound_result[i].buffer);
        }
        free(m_bound_result);
        free(m_out_null);
        free(m_out_length);
      }

      column_count = (int)mysql_num_fields(m_result);
      m_bound_result = (MYSQL_BIND*)calloc(column_count, sizeof(MYSQL_BIND));
      m_out_null = (my_bool*)calloc(column_count, sizeof(my_bool));
      m_out_length = (unsigned long *)calloc(column_count,
                                             sizeof(unsigned long));

      /* summon memory to hold the row */
      for (i = 0; i < column_count; i++) {
        if (calc_max_length && m_fields[i].type == FIELD_TYPE_BLOB) {
          my_bool on = 1;
          mysql_stmt_attr_set(m_stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &on);
          calc_max_length = 0;
        }
        switch (m_fields[i].type) {
          case FIELD_TYPE_INT24:
            m_bound_result[i].buffer_length = MAX_MEDIUMINT_WIDTH + 1;
            break;
          case FIELD_TYPE_LONG:
            m_bound_result[i].buffer_length = MAX_INT_WIDTH + 1;
            break;
          case FIELD_TYPE_LONGLONG:
            m_bound_result[i].buffer_length = MAX_BIGINT_WIDTH + 1;
            break;
          case FIELD_TYPE_TINY:
            m_bound_result[i].buffer_length = MAX_TINYINT_WIDTH + 1;
            break;
          case FIELD_TYPE_SHORT:
            m_bound_result[i].buffer_length = MAX_SMALLINT_WIDTH + 1;
            break;
          default:
            m_bound_result[i].buffer_length =
              m_fields[i].max_length? m_fields[i].max_length:
              m_fields[i].length;
            /* work-around for longtext and alike */
            if (m_bound_result[i].buffer_length > m_conn->max_buffer_size()) {
              m_bound_result[i].buffer_length = m_conn->max_buffer_size();
            }
        }

        /* there are cases where the length reported by mysql is too short.
         * eg: when describing a table that contains an enum column. Since
         * we have no way of knowing the true length either, we'll bump up
         * our buffer size to a reasonable size, just in case */
        if (m_fields[i].max_length == 0 &&
            m_bound_result[i].buffer_length < 128 && MYSQL_TYPE_VAR_STRING) {
          m_bound_result[i].buffer_length = 128;
        }

        m_out_length[i] = 0;

        m_bound_result[i].buffer = malloc(m_bound_result[i].buffer_length);
        m_bound_result[i].is_null = &m_out_null[i];
        m_bound_result[i].length = &m_out_length[i];
        m_bound_result[i].buffer_type = MYSQL_TYPE_STRING;
      }

      if (mysql_stmt_bind_result(m_stmt, m_bound_result)) {
        handleError(__FILE__, __LINE__);
        return false;
      }

      /* if buffered, pre-fetch all the data */
      if (m_conn->buffered()) {
        mysql_stmt_store_result(m_stmt);
      }
    }
  }

  setRowCount();
  return true;
}

static const char *type_to_name_native(int type) {
#define PDO_MYSQL_NATIVE_TYPE_NAME(x)  case FIELD_TYPE_##x: return #x;

  switch (type) {
        PDO_MYSQL_NATIVE_TYPE_NAME(STRING)
        PDO_MYSQL_NATIVE_TYPE_NAME(VAR_STRING)
#ifdef MYSQL_HAS_TINY
        PDO_MYSQL_NATIVE_TYPE_NAME(TINY)
#endif
        PDO_MYSQL_NATIVE_TYPE_NAME(SHORT)
        PDO_MYSQL_NATIVE_TYPE_NAME(LONG)
        PDO_MYSQL_NATIVE_TYPE_NAME(LONGLONG)
        PDO_MYSQL_NATIVE_TYPE_NAME(INT24)
        PDO_MYSQL_NATIVE_TYPE_NAME(FLOAT)
        PDO_MYSQL_NATIVE_TYPE_NAME(DOUBLE)
        PDO_MYSQL_NATIVE_TYPE_NAME(DECIMAL)
#ifdef FIELD_TYPE_NEWDECIMAL
        PDO_MYSQL_NATIVE_TYPE_NAME(NEWDECIMAL)
#endif
#ifdef FIELD_TYPE_GEOMETRY
        PDO_MYSQL_NATIVE_TYPE_NAME(GEOMETRY)
#endif
        PDO_MYSQL_NATIVE_TYPE_NAME(TIMESTAMP)
#ifdef MYSQL_HAS_YEAR
        PDO_MYSQL_NATIVE_TYPE_NAME(YEAR)
#endif
        PDO_MYSQL_NATIVE_TYPE_NAME(SET)
        PDO_MYSQL_NATIVE_TYPE_NAME(ENUM)
        PDO_MYSQL_NATIVE_TYPE_NAME(DATE)
#ifdef FIELD_TYPE_NEWDATE
        PDO_MYSQL_NATIVE_TYPE_NAME(NEWDATE)
#endif
        PDO_MYSQL_NATIVE_TYPE_NAME(TIME)
        PDO_MYSQL_NATIVE_TYPE_NAME(DATETIME)
        PDO_MYSQL_NATIVE_TYPE_NAME(TINY_BLOB)
        PDO_MYSQL_NATIVE_TYPE_NAME(MEDIUM_BLOB)
        PDO_MYSQL_NATIVE_TYPE_NAME(LONG_BLOB)
        PDO_MYSQL_NATIVE_TYPE_NAME(BLOB)
        PDO_MYSQL_NATIVE_TYPE_NAME(NULL)
  default:
    return NULL;
  }
#undef PDO_MYSQL_NATIVE_TYPE_NAME
}

///////////////////////////////////////////////////////////////////////////////

PDOMySqlStatement::PDOMySqlStatement(PDOMySqlConnection *conn, MYSQL *server)
    : m_conn(conn), m_server(server), m_result(NULL), m_fields(NULL),
      m_current_data(NULL), m_current_lengths(NULL), m_stmt(NULL),
      m_num_params(0), m_params(NULL), m_in_null(NULL), m_in_length(NULL),
      m_bound_result(NULL), m_out_null(NULL), m_out_length(NULL),
      m_params_given(0), m_max_length(0) {
  this->dbh = conn;
}

PDOMySqlStatement::~PDOMySqlStatement() {
  sweep();
}

void PDOMySqlStatement::sweep() {
  if (m_result) {
    /* free the resource */
    mysql_free_result(m_result);
    m_result = NULL;
  }
  if (m_einfo.errmsg) {
    free(m_einfo.errmsg);
    m_einfo.errmsg = NULL;
  }
  if (m_stmt) {
    mysql_stmt_close(m_stmt);
    m_stmt = NULL;
  }

  if (m_params) {
    free(m_params);
  }
  if (m_in_null) {
    free(m_in_null);
  }
  if (m_in_length) {
    free(m_in_length);
  }

  if (m_bound_result) {
    int i;
    for (i = 0; i < column_count; i++) {
      free(m_bound_result[i].buffer);
    }

    free(m_bound_result);
    free(m_out_null);
    free(m_out_length);
  }

  if (m_server) {
    while (mysql_more_results(m_server)) {
      if (mysql_next_result(m_server) != 0) {
        break;
      }
      MYSQL_RES *res = mysql_store_result(m_server);
      if (res) {
        mysql_free_result(res);
      }
    }
  }
}

bool PDOMySqlStatement::create(const String& sql, const Array& options) {
  supports_placeholders = PDO_PLACEHOLDER_POSITIONAL;

  String nsql;
  int ret = pdo_parse_params(this, sql, nsql);
  if (ret == 1) {
    /* query was rewritten */
  } else if (ret == -1) {
    /* failed to parse */
    return false;
  } else {
    nsql = sql;
  }

  if (!(m_stmt = mysql_stmt_init(m_server))) {
    handleError(__FILE__, __LINE__);
    return false;
  }

  if (mysql_stmt_prepare(m_stmt, nsql.data(), nsql.size())) {
    /* TODO: might need to pull statement specific info here? */
    /* if the query isn't supported by the protocol, fallback to emulation */
    if (mysql_errno(m_server) == 1295) {
      supports_placeholders = PDO_PLACEHOLDER_NONE;
      return true;
    }
    handleError(__FILE__, __LINE__);
    return false;
  }

  m_num_params = mysql_stmt_param_count(m_stmt);
  if (m_num_params) {
    m_params_given = 0;
    m_params = (MYSQL_BIND*)calloc(m_num_params, sizeof(MYSQL_BIND));
    m_in_null = (my_bool*)calloc(m_num_params, sizeof(my_bool));
    m_in_length = (unsigned long*)calloc(m_num_params, sizeof(unsigned long));
  }

  m_max_length = pdo_attr_lval(options, PDO_ATTR_MAX_COLUMN_LEN, 0);
  return true;
}

bool PDOMySqlStatement::support(SupportedMethod method) {
  switch (method) {
  case MethodSetAttribute:
  case MethodGetAttribute:
    return false;
  default:
    break;
  }
  return true;
}

int PDOMySqlStatement::handleError(const char *file, int line) {
  assert(m_conn);
  return m_conn->handleError(file, line, this);
}

bool PDOMySqlStatement::executer() {
  if (m_stmt) {
    return executePrepared();
  }

  /* ensure that we free any previous unfetched results */
  if (m_result) {
    mysql_free_result(m_result);
    m_result = NULL;
  }

  if (mysql_real_query(m_server, active_query_string.data(),
                       active_query_string.size()) != 0) {
    handleError(__FILE__, __LINE__);
    return false;
  }

  my_ulonglong affected_count = mysql_affected_rows(m_server);
  if (affected_count == (my_ulonglong)-1) {
    /* we either have a query that returned a result set or an error occured
       lets see if we have access to a result set */
    if (!m_conn->buffered()) {
      m_result = mysql_use_result(m_server);
    } else {
      m_result = mysql_store_result(m_server);
    }
    if (NULL == m_result) {
      handleError(__FILE__, __LINE__);
      return false;
    }

    row_count = mysql_num_rows(m_result);
    column_count = (int) mysql_num_fields(m_result);
    m_fields = mysql_fetch_fields(m_result);

  }
  else {
    row_count = affected_count;
  }

  return true;
}

bool PDOMySqlStatement::fetcher(PDOFetchOrientation ori, long offset) {
  int ret;
  if (m_stmt) {
    ret = mysql_stmt_fetch(m_stmt);
    if (ret == MYSQL_DATA_TRUNCATED) {
      ret = 0;
    }
    if (ret) {
      if (ret != MYSQL_NO_DATA) {
        handleError(__FILE__, __LINE__);
      }
      return false;
    }
    return true;
  }

  if (!m_result) {
    strcpy(error_code, "HY000");
    return false;
  }

  if ((m_current_data = mysql_fetch_row(m_result)) == NULL) {
    if (mysql_errno(m_server)) {
      handleError(__FILE__, __LINE__);
    }
    return false;
  }

  m_current_lengths = (long int *)mysql_fetch_lengths(m_result);
  return true;
}

bool PDOMySqlStatement::describer(int colno) {
  if (!m_result) {
    return false;
  }

  if (colno < 0 || colno >= column_count) {
    /* error invalid column */
    return false;
  }

  if (columns.empty()) {
    for (int i = 0; i < column_count; i++) {
      columns.set(i, Resource(NEWOBJ(PDOColumn)));
    }
  }

  // fetch all on demand, this seems easiest if we've been here before bail out
  PDOColumn *col = columns[0].toResource().getTyped<PDOColumn>();
  if (!col->name.empty()) {
    return true;
  }
  for (int i = 0; i < column_count; i++) {
    col = columns[i].toResource().getTyped<PDOColumn>();

    if (m_conn->fetch_table_names()) {
      col->name = String(m_fields[i].table) + "." +
        String(m_fields[i].name);
    } else {
      col->name = String(m_fields[i].name, CopyString);
    }

    col->precision = m_fields[i].decimals;
    col->maxlen = m_fields[i].length;
    col->param_type = PDO_PARAM_STR;
  }
  return true;
}

bool PDOMySqlStatement::getColumn(int colno, Variant &value) {
  if (!m_result) {
    return false;
  }

  if (!m_stmt) {
    if (m_current_data == NULL || !m_result) {
      return false;
    }
  }
  if (colno < 0 || colno >= column_count) {
    /* error invalid column */
    return false;
  }
  char *ptr; int len;
  if (m_stmt) {
    if (m_out_null[colno]) {
      value = String();
      return true;
    }
    ptr = (char*)m_bound_result[colno].buffer;
    if (m_out_length[colno] > m_bound_result[colno].buffer_length) {
      /* mysql lied about the column width */
      strcpy(error_code, "01004"); /* truncated */
      m_out_length[colno] = m_bound_result[colno].buffer_length;
      len = m_out_length[colno];
      value = String(ptr, len, CopyString);
      return false;
    }
    len = m_out_length[colno];
    value = String(ptr, len, CopyString);
    return true;
  }
  ptr = m_current_data[colno];
  len = m_current_lengths[colno];
  value = String(ptr, len, CopyString);
  return true;
}

bool PDOMySqlStatement::paramHook(PDOBoundParam *param,
                                  PDOParamEvent event_type) {
  MYSQL_BIND *b;
  if (m_stmt && param->is_param) {
    switch (event_type) {
    case PDO_PARAM_EVT_ALLOC:
      /* sanity check parameter number range */
      if (param->paramno < 0 || param->paramno >= m_num_params) {
        strcpy(error_code, "HY093");
        return false;
      }
      m_params_given++;

      b = &m_params[param->paramno];
      param->driver_data = b;
      b->is_null = &m_in_null[param->paramno];
      b->length = &m_in_length[param->paramno];
      /* recall how many parameters have been provided */
      return true;

    case PDO_PARAM_EVT_EXEC_PRE:
      if ((int)m_params_given < m_num_params) {
        /* too few parameter bound */
        strcpy(error_code, "HY093");
        return false;
      }

      b = (MYSQL_BIND*)param->driver_data;
      *b->is_null = 0;
      if (PDO_PARAM_TYPE(param->param_type) == PDO_PARAM_NULL ||
          param->parameter.isNull()) {
        *b->is_null = 1;
        b->buffer_type = MYSQL_TYPE_STRING;
        b->buffer = NULL;
        b->buffer_length = 0;
        *b->length = 0;
        return true;
      }

      switch (PDO_PARAM_TYPE(param->param_type)) {
      case PDO_PARAM_STMT:
        return false;
      case PDO_PARAM_LOB:
        if (param->parameter.isResource()) {
          Variant buf = f_stream_get_contents(param->parameter.toResource());
          if (!same(buf, false)) {
            param->parameter = buf;
          } else {
            pdo_raise_impl_error(m_conn, this, "HY105",
                                 "Expected a stream resource");
            return false;
          }
        }
        /* fall through */

      default:
        ;
      }

      if (param->parameter.isString()) {
        String sparam = param->parameter.toString();
        b->buffer_type = MYSQL_TYPE_STRING;
        b->buffer = (void*)sparam.data();
        b->buffer_length = sparam.size();
        *b->length = sparam.size();
        return true;
      }
      if (param->parameter.isInteger()) {
        param->parameter = param->parameter.toInt64();
        b->buffer_type = MYSQL_TYPE_LONG;
        b->buffer = param->parameter.getInt64Data();
        return true;
      }
      if (param->parameter.isDouble()) {
        b->buffer_type = MYSQL_TYPE_DOUBLE;
        b->buffer = param->parameter.getDoubleData();
        return true;
      }
      return false;
    case PDO_PARAM_EVT_FREE:
    case PDO_PARAM_EVT_EXEC_POST:
    case PDO_PARAM_EVT_FETCH_PRE:
    case PDO_PARAM_EVT_FETCH_POST:
    case PDO_PARAM_EVT_NORMALIZE:
      /* do nothing */
      break;
    }
  }
  return true;
}

const StaticString
  s_mysql_def("mysql:def"),
  s_not_null("not_null"),
  s_primary_key("primary_key"),
  s_multiple_key("multiple_key"),
  s_unique_key("unique_key"),
  s_blob("blob"),
  s_native_type("native_type"),
  s_flags("flags"),
  s_table("table");

bool PDOMySqlStatement::getColumnMeta(int64_t colno, Array &ret) {
  if (!m_result) {
    return false;
  }
  if (colno < 0 || colno >= column_count) {
    /* error invalid column */
    return false;
  }

  Array flags = Array::Create();

  const MYSQL_FIELD *F = m_fields + colno;
  if (F->def) {
    ret.set(s_mysql_def, String(F->def, CopyString));
  }
  if (IS_NOT_NULL(F->flags)) {
    flags.append(s_not_null);
  }
  if (IS_PRI_KEY(F->flags)) {
    flags.append(s_primary_key);
  }
  if (F->flags & MULTIPLE_KEY_FLAG) {
    flags.append(s_multiple_key);
  }
  if (F->flags & UNIQUE_KEY_FLAG) {
    flags.append(s_unique_key);
  }
  if (IS_BLOB(F->flags)) {
    flags.append(s_blob);
  }
  const char *str = type_to_name_native(F->type);
  if (str) {
    ret.set(s_native_type, str);
  }
  ret.set(s_flags, flags);
  ret.set(s_table, String(F->table, CopyString));
  return true;
}

bool PDOMySqlStatement::nextRowset() {
  /* ensure that we free any previous unfetched results */
  if (m_stmt) {
    column_count = (int)mysql_num_fields(m_result);
    mysql_stmt_free_result(m_stmt);
  }
  if (m_result) {
    mysql_free_result(m_result);
    m_result = NULL;
  }

  int ret = mysql_next_result(m_server);
  if (ret > 0) {
    handleError(__FILE__, __LINE__);
    return false;
  }
  if (ret < 0) {
    /* No more results */
    return false;
  }

  my_ulonglong row_count;
  if (!m_conn->buffered()) {
    m_result = mysql_use_result(m_server);
    row_count = 0;
  } else {
    m_result = mysql_store_result(m_server);
    if ((my_ulonglong)-1 == (row_count = mysql_affected_rows(m_server))) {
      handleError(__FILE__, __LINE__);
      return false;
    }
  }

  if (!m_result) {
    return false;
  }

  column_count = (int)mysql_num_fields(m_result);
  m_fields = mysql_fetch_fields(m_result);
  return true;
}

bool PDOMySqlStatement::cursorCloser() {
  if (m_result) {
    mysql_free_result(m_result);
    m_result = NULL;
  }
  if (m_stmt) {
    return !mysql_stmt_free_result(m_stmt);
  }

  while (mysql_more_results(m_server)) {
    if (mysql_next_result(m_server) != 0) {
      break;
    }
    MYSQL_RES *res = mysql_store_result(m_server);
    if (res) {
      mysql_free_result(res);
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

PDOMySql::PDOMySql() : PDODriver("mysql") {
}

PDOConnection *PDOMySql::createConnectionObject() {
  // Doesn't use NEWOBJ because PDOConnection is malloced
  return new PDOMySqlConnection();
}

///////////////////////////////////////////////////////////////////////////////
}
