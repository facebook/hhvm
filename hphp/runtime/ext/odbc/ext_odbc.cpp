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

#ifdef HAVE_UODBC

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#define MAX_COLUMN_NAME   256
#define MAX_ERROR_MSG     256

// TODO: we should probably implement a fancier buffer size control here.
// At a first sight, a 100 rows at a time seemed a good start point.
#define PER_FETCH_ROWS    100

namespace HPHP {

template<class T>
static req::ptr<T> safe_cast(const Resource& res) {
  auto ptr = dyn_cast_or_null<T>(res);
  if (!ptr) {
    raise_warning("supplied argument is not a valid ODBC resource");
  }
  return ptr;
}

///////////////////////////////////////////////////////////////////////////////
class ODBCContext {
public:
  // extract an error from a handle
  static void extract_error(const SQLSMALLINT type, const SQLHANDLE hdl);

  // return the last error msg extracted
  static SQLCHAR* get_last_error_msg();

  // return the last error code extracted
  static SQLCHAR* get_last_error_code();

private:
  // stores the last diagnostic message
  static SQLCHAR error_msg_[MAX_ERROR_MSG];

  // 6-digit ODBC state
  static SQLCHAR error_sql_[8];

  // the native error code, specific to the data source
  static SQLINTEGER error_native_;

  // preventing instantiation
  ODBCContext() {};
};
SQLCHAR ODBCContext::error_msg_[MAX_ERROR_MSG];
SQLCHAR ODBCContext::error_sql_[8];
SQLINTEGER ODBCContext::error_native_;

// extracts the error msg from a particular handle
void ODBCContext::extract_error(const SQLSMALLINT type, const SQLHANDLE hdl)
{
  // TODO: we're only saving the last error
  SQLSMALLINT len = -1;
  SQLSMALLINT i = 0;

  // skipping to the last error we have
  while (SQLGetDiagRec(type, hdl, ++i, ODBCContext::error_sql_,
          &ODBCContext::error_native_, ODBCContext::error_msg_,
          sizeof(ODBCContext::error_msg_), &len) == SQL_SUCCESS) {

    // some drivers return a a new line at the end of the msg
    if (len > 0 && ODBCContext::error_msg_[len-1] == '\n') {
      ODBCContext::error_msg_[len-1] = '\0';
    }
  }
}

SQLCHAR* ODBCContext::get_last_error_code()
{
  return ODBCContext::error_sql_;
}

SQLCHAR* ODBCContext::get_last_error_msg()
{
  return ODBCContext::error_msg_;
}

///////////////////////////////////////////////////////////////////////////////

class ODBCColumn : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(ODBCColumn);

  explicit ODBCColumn(const SQLHSTMT hdl_stmt, const int i_col);

  // return the column name
  const String get_name() const;

  // return the size of the data buffer needed by this column
  SQLLEN total_data_size() const;

  // return the size of the whole buffer needed by this column
  SQLLEN total_column_size() const;

  ~ODBCColumn();

private:
  SQLCHAR column_name[MAX_COLUMN_NAME];
  SQLSMALLINT buffer_len;
  SQLSMALLINT name_length;
  SQLSMALLINT sql_data_type;
  SQLSMALLINT c_data_type;
  SQLULEN column_size;
  SQLSMALLINT decimal_digits;
  SQLSMALLINT nullable;
};
ODBCColumn::~ODBCColumn() {}
void ODBCColumn::sweep() {}

ODBCColumn::ODBCColumn(const SQLHSTMT hdl_stmt, const int i_col)
{
  column_name[0] = '\0';
  buffer_len = sizeof(column_name);
  name_length = 0;
  sql_data_type = 0;
  c_data_type = 0;
  column_size = 0;
  decimal_digits = 0;
  nullable = 0;

  // retrieve column information from odbc
  if (!SQL_SUCCEEDED(
        SQLDescribeCol(hdl_stmt, i_col, column_name, buffer_len, &name_length,
          &sql_data_type, &column_size, &decimal_digits, &nullable))) {
      ODBCContext::extract_error(SQL_HANDLE_STMT, hdl_stmt);
      raise_warning("unable to retrieve column information.");
  }

  // figure out the best c data type based on the sql data type we have
  c_data_type = SQL_C_CHAR;
  switch (sql_data_type) {
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_TINYINT:
    case SQL_BIGINT:
      c_data_type = SQL_C_LONG;
      break;

    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
      c_data_type = SQL_C_DOUBLE;
      break;

    case SQL_CHAR:
    case SQL_VARCHAR:
    // TODO: we're returning timestamps as strings
    case SQL_TYPE_TIMESTAMP:
    default:
      c_data_type = SQL_C_CHAR;
      break;
  }
}
const String ODBCColumn::get_name() const
{
  return String::FromCStr((char*)column_name);
}
SQLLEN ODBCColumn::total_data_size() const
{
  SQLULEN needed = column_size;

  // room for '\0' termination byte
  if (c_data_type == SQL_C_CHAR) {
    needed++;
  }

  // aligning for 64 bits
  if (needed % 8 != 0) {
    return (needed / 8 + 1) * 8;
  }
  return needed;
}
SQLLEN ODBCColumn::total_column_size() const
{
  // data size + status
  return total_data_size() + sizeof(SQLLEN);
}

///////////////////////////////////////////////////////////////////////////////

class ODBCParam : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(ODBCParam);

  ODBCParam(const SQLHSTMT hdl_stmt, const int i_col);

  SQLSMALLINT type, decimal, nullable;
  SQLULEN col_size;
};
IMPLEMENT_RESOURCE_ALLOCATION(ODBCParam);

ODBCParam::ODBCParam(const SQLHSTMT hdl_stmt, const int i_col)
{
  type = 0;
  decimal = 0;
  nullable = 0;
  col_size = 0;

  if (!SQL_SUCCEEDED(
        SQLDescribeParam(hdl_stmt, i_col, &type,
          &col_size, &decimal, &nullable))) {
    ODBCContext::extract_error(SQL_HANDLE_STMT, hdl_stmt);
    raise_warning("unable to retrieve param information.");
  }
}

///////////////////////////////////////////////////////////////////////////////

// created per-query
class ODBCCursor : public SweepableResourceData {
public:
  CLASSNAME_IS("odbc cursor")
  const String& o_getClassNameHook() const override { return classnameof(); }
  DECLARE_RESOURCE_ALLOCATION(ODBCCursor);

  // needs a connection
  explicit ODBCCursor(SQLHDBC hdl_dbconn);
  ~ODBCCursor();

  // executes a query and open the cursor
  bool exec_query(const String& query);

  // prepares a query for further execution
  bool prepare_query(const String& query);

  // executes a previously prepared query
  bool exec_prepared_query(const Array params);

  // fetch one row of the cursor
  Variant fetch();

  // return the number of the rows in the resultset
  int64_t num_rows() const;

private:
  // fetch more rows into the buffers
  int fetch_more_rows();

  // allocate and bind the buffer
  bool bind_buffer();

  // retrieve the number of column in the current resultset
  void set_num_cols();

  // number of rows to fetch per ODBC call
  SQLLEN per_fetch_rows = PER_FETCH_ROWS;

  // number of rows left inside the buffer
  int rows_in_buffer_;

  // buffer itself
  SQLPOINTER buffer_;

  // current position inside buffer
  SQLPOINTER buffer_cursor_;

  // number of rows returned by the ODBC call
  SQLULEN num_rows_fetched_;

  // is the buffer bound already?
  bool is_buffer_bound;

  // store columns info
  Array columns_;

  // number of columns
  SQLSMALLINT columns_count_;

  // store parameters info (prepard stmts only)
  Array params_;

  // number of params (prepared stmts only)
  SQLSMALLINT params_size_;

  // statement handler
  SQLHSTMT hdl_stmt_;
};

ODBCCursor::ODBCCursor(SQLHDBC hdl_dbconn)
{
  params_size_ = 0;
  columns_count_ = 0;
  is_buffer_bound = false;
  rows_in_buffer_ = 0;
  if (!SQL_SUCCEEDED(
        SQLAllocHandle(SQL_HANDLE_STMT, hdl_dbconn, &hdl_stmt_))) {
    ODBCContext::extract_error(SQL_HANDLE_DBC, hdl_dbconn);
  } else {
    intptr_t timeout = ThreadInfo::s_threadInfo->
      m_reqInjectionData.getRemainingTime();
    if (timeout) {
      if (!SQL_SUCCEEDED(
            SQLSetStmtAttr(hdl_stmt_,
                           SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)timeout,
                           0))) {
        ODBCContext::extract_error(SQL_HANDLE_DBC, hdl_stmt_);
      }
    }
  }
}

ODBCCursor::~ODBCCursor() {
  SQLFreeStmt(hdl_stmt_, SQL_CLOSE);
  SQLFreeHandle(SQL_HANDLE_STMT, hdl_stmt_);
  if (is_buffer_bound) {
    req::free(buffer_);
  }
}

void ODBCCursor::sweep()
{
  SQLFreeStmt(hdl_stmt_, SQL_CLOSE);
  SQLFreeHandle(SQL_HANDLE_STMT, hdl_stmt_);
  if (is_buffer_bound) {
    req::free(buffer_);
  }
}

bool ODBCCursor::exec_query(const String& query)
{
  SYNC_VM_REGS_SCOPED();

  if (!SQL_SUCCEEDED(
        SQLExecDirect(hdl_stmt_, (SQLCHAR*)query.c_str(), SQL_NTS))) {
    ODBCContext::extract_error(SQL_HANDLE_STMT, hdl_stmt_);
    return false;
  }
  set_num_cols();
  return true;
}

bool ODBCCursor::prepare_query(const String& query)
{
  if (!SQL_SUCCEEDED(
        SQLPrepare(hdl_stmt_, (SQLCHAR*)query.c_str(), SQL_NTS))) {
    ODBCContext::extract_error(SQL_HANDLE_STMT, hdl_stmt_);
    return false;
  }

  // store the number of parameters in this stmt
  if (!SQL_SUCCEEDED(
        SQLNumParams(hdl_stmt_, &params_size_))) {
    ODBCContext::extract_error(SQL_HANDLE_STMT, hdl_stmt_);
    return false;
  }

  // store information about data types the db is expecting
  for (int i=1; i <= params_size_; i++) {
    params_.append(Variant(req::make<ODBCParam>(hdl_stmt_, i)));
  }
  assert(params_.size() == params_size_);
  return true;
}

bool ODBCCursor::exec_prepared_query(const Array params)
{
  SYNC_VM_REGS_SCOPED();

  SQLCHAR* input[params_size_];
  int64_t num_rows;

  for (int i=0; i < params_size_; i++) {
    const Array &cur_array = params[i].toArray();
    auto param = cast<ODBCParam>(params_[i]);
    num_rows = cur_array.size();

    // allocate buffer we'll pass to odbc
    input[i] = (SQLCHAR*)req::malloc(num_rows * param->col_size);

    // copy each element of our input array to the buffer
    for (int j=0; j < num_rows; j++) {
      strcpy((char*)(input[i] + (j * param->col_size)),
        cur_array[j].toString().c_str());
    }

    // bind data arrays to the parameters in the prepared query
    if (!SQL_SUCCEEDED(
          SQLBindParameter(hdl_stmt_, i+1, SQL_PARAM_INPUT, SQL_C_CHAR,
            param->type, param->col_size, param->decimal,
            (SQLPOINTER)input[i], param->col_size, nullptr))) {
      ODBCContext::extract_error(SQL_HANDLE_STMT, hdl_stmt_);
      return false;
    }
  }

  // tell the ODBC driver how many rows we have in the array.
  if (!SQL_SUCCEEDED(
        SQLSetStmtAttr(hdl_stmt_, SQL_ATTR_PARAMSET_SIZE,
          (SQLPOINTER)num_rows, 0))) {
    ODBCContext::extract_error(SQL_HANDLE_STMT, hdl_stmt_);
    return false;
  }

  // execute the stmt itself
  if (!SQL_SUCCEEDED(
        SQLExecute(hdl_stmt_))) {
    ODBCContext::extract_error(SQL_HANDLE_STMT, hdl_stmt_);
    return false;
  }

  for (int i=0; i < params_size_; i++) {
    req::free(input[i]);
  }

  set_num_cols();
  return true;
}

bool ODBCCursor::bind_buffer()
{
  SQLLEN row_size = 0;

  // retrieve info about columns
  for (int i_col=1; i_col <= columns_count_; i_col++) {
    auto column = req::make<ODBCColumn>(hdl_stmt_, i_col);
    row_size += column->total_column_size();
    columns_.append(Variant(std::move(column)));
  }
  buffer_ = (SQLPOINTER)req::malloc(row_size * per_fetch_rows);

  // since this buffer can be quite big
  if (buffer_ == nullptr) {
    raise_warning("Unable to allocate buffer.");
    return false;
  }

  // specifies row-wise and the size of a single row
  SQLSetStmtAttr(hdl_stmt_, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)row_size, 0);

  // specifies the number of rows returned in a single fetch call
  SQLSetStmtAttr(hdl_stmt_, SQL_ATTR_ROW_ARRAY_SIZE,
      (SQLPOINTER)per_fetch_rows, 0);

  // we won't use per-row status
  SQLSetStmtAttr(hdl_stmt_, SQL_ATTR_ROW_STATUS_PTR, nullptr, 0);

  // where to return number of rows fetched
  SQLSetStmtAttr(hdl_stmt_, SQL_ATTR_ROWS_FETCHED_PTR, &num_rows_fetched_, 0);

  SQLPOINTER cursor = buffer_;
  SQLLEN buffer_len;
  for (int i=0; i < columns_count_; i++) {
    auto column = cast<ODBCColumn>(columns_[i]);
    buffer_len = column->total_data_size();

    // TODO - we should not get every data type as a SQL_C_CHAR, but
    // cast to an appropriate type instead
    SQLBindCol(hdl_stmt_, i+1, SQL_C_CHAR, cursor, buffer_len,
        (SQLLEN*)((char*)cursor + buffer_len));

    cursor = (SQLPOINTER)((char*)cursor + column->total_column_size());
  }
  assert(columns_count_ == columns_.size());
  is_buffer_bound = true;
  return true;
}

void ODBCCursor::set_num_cols()
{
  // check the number of columns in resultset
  if (!SQL_SUCCEEDED(
        SQLNumResultCols(hdl_stmt_, &columns_count_))) {
    ODBCContext::extract_error(SQL_HANDLE_STMT, hdl_stmt_);
    columns_count_ = 0;
  }
}

int ODBCCursor::fetch_more_rows()
{
  SQLRETURN ret;

  // check if the buffers are already bound
  if (!is_buffer_bound) {
    bind_buffer();
  }

  ret = SQLFetch(hdl_stmt_);
  if (ret == SQL_NO_DATA_FOUND) {
    return 0;
  }
  else if (!SQL_SUCCEEDED(ret)) {
    ODBCContext::extract_error(SQL_HANDLE_STMT, hdl_stmt_);
    return 0;
  }
  // num_rows_fetched's address was bound before
  rows_in_buffer_ = num_rows_fetched_;

  // point to buffer's start
  buffer_cursor_ = buffer_;
  return num_rows_fetched_;
}

Variant ODBCCursor::fetch()
{
  // check whether this stmt generates a resultset
  if (!columns_count_) {
    return false;
  }

  // if we dont have buffered rows, fetch more
  if (rows_in_buffer_ < 1) {
    if (fetch_more_rows() < 1) {
      return false;
    }
  }

  Array ret_data;
  SQLLEN status;

  // create an entry for each column of the resultset
  for (int i=0; i != columns_count_; i++) {
    auto column = cast<ODBCColumn>(columns_[i]);
    status = *((char*)buffer_cursor_ + column->total_data_size());

    // status holds the string size, or SQL_NULL_DATA
    if (status == SQL_NULL_DATA) {
      ret_data.set(column->get_name(), String());
    } else {
      ret_data.set(column->get_name(),
          String::FromCStr((char*)buffer_cursor_));
    }

    // walking to next column
    buffer_cursor_ =
        (SQLPOINTER)((char*)buffer_cursor_ + column->total_column_size());
  }
  rows_in_buffer_--;
  return ret_data;
}

int64_t ODBCCursor::num_rows() const
{
  SQLLEN rows;

  if (!SQL_SUCCEEDED(
        SQLRowCount(hdl_stmt_, &rows))) {
    ODBCContext::extract_error(SQL_HANDLE_STMT, hdl_stmt_);
    return -1;
  }
  return (int64_t)rows;
}

///////////////////////////////////////////////////////////////////////////////
class ODBCLink : public SweepableResourceData {
public:
  CLASSNAME_IS("odbc link")
  const String& o_getClassNameHook() const override { return classnameof(); }
  DECLARE_RESOURCE_ALLOCATION(ODBCLink);

  ODBCLink();
  ~ODBCLink();

  // tries to connect to the database
  bool connect(const String& dsn, const String& username,
      const String& password);

  // closes the connection an frees structures
  void close();

  // executes a query and returns an ODBCCursor obj
  Variant exec(const String& query);

  // prepares a query for further binding and execution
  Variant prepare(const String& query);

  // toggle autocommit behavior on/off
  bool set_autocommit(const bool on_off);

  // get current autocommit behavior
  bool get_autocommit() const;

  // commit the current transaction
  bool commit();

  // rollback the current transaction
  bool rollback();

private:
  // env handler
  SQLHENV hdl_env_;

  // connection handler
  SQLHDBC hdl_dbconn_;

  // finish current transaction (commit or rollback)
  bool end_transaction(const bool is_commit);
};
IMPLEMENT_RESOURCE_ALLOCATION(ODBCLink);

ODBCLink::ODBCLink()
{
  // allocate env handle.
  if (!SQL_SUCCEEDED(
        SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hdl_env_))) {
    ODBCContext::extract_error(SQL_HANDLE_ENV, hdl_env_);
  }

  // Set the ODBC version we are going to use to 3.
  if (!SQL_SUCCEEDED(
        SQLSetEnvAttr(hdl_env_, SQL_ATTR_ODBC_VERSION,
          (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_UINTEGER))) {
    ODBCContext::extract_error(SQL_HANDLE_ENV, hdl_env_);
  }

  // allocate a database handle.
  if (!SQL_SUCCEEDED(
        SQLAllocHandle(SQL_HANDLE_DBC, hdl_env_, &hdl_dbconn_))) {
    ODBCContext::extract_error(SQL_HANDLE_DBC, hdl_dbconn_);
  }
}

ODBCLink::~ODBCLink() {
  close();
}

bool ODBCLink::connect(const String& dsn, const String& username,
    const String& password)
{

  // if we find a semicolon, then it's not just a DSN
  if (dsn.find(';') != -1) {
    String conn_str(dsn);

    // if we don't have a UID, try to append it
    if ((dsn.find("UID", 0, false) == -1) && !username.empty()) {
      conn_str += ";UID=";
      conn_str += username;
    }

    // if we don't have a PWD, try to append it
    if ((dsn.find("PWD", 0, false) == -1) && !password.empty()) {
      conn_str += ";PWD=";
      conn_str += password;
    }

    if (!SQL_SUCCEEDED(
          SQLDriverConnect(hdl_dbconn_, nullptr, (SQLCHAR*)conn_str.c_str(),
            SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_COMPLETE))) {
      ODBCContext::extract_error(SQL_HANDLE_DBC, hdl_dbconn_);
      return false;
    }
  }
  // using dsn
  else {

    // connecting to the database
    if (!SQL_SUCCEEDED(
          SQLConnect(hdl_dbconn_, (SQLCHAR*)dsn.c_str(), SQL_NTS,
            (SQLCHAR*)username.c_str(), SQL_NTS,
            (SQLCHAR*)password.c_str(), SQL_NTS))) {
      ODBCContext::extract_error(SQL_HANDLE_DBC, hdl_dbconn_);
      return false;
    }
  }
  return true;
}

void ODBCLink::close()
{
  SQLDisconnect(hdl_dbconn_);
  SQLFreeHandle(SQL_HANDLE_DBC, hdl_dbconn_);
  SQLFreeHandle(SQL_HANDLE_ENV, hdl_env_);
}

Variant ODBCLink::exec(const String& query)
{
  auto cursor = req::make<ODBCCursor>(hdl_dbconn_);

  if (!cursor->exec_query(query)) {
    raise_warning("SQL error: [%s] %s",
      (char*)ODBCContext::get_last_error_code(),
      (char*)ODBCContext::get_last_error_msg());
    return false;
  }
  return Variant(std::move(cursor));
}

Variant ODBCLink::prepare(const String& query)
{
  auto cursor = req::make<ODBCCursor>(hdl_dbconn_);

  if (!cursor->prepare_query(query)) {
    return false;
  }
  return Variant(std::move(cursor));
}

bool ODBCLink::set_autocommit(const bool on_off)
{
  SQLPOINTER value = SQL_AUTOCOMMIT_OFF;

  if (on_off) {
    value = (SQLPOINTER)SQL_AUTOCOMMIT_ON;
  }

  // toggle AUTOCOMMIT status
  if (!SQL_SUCCEEDED(
          SQLSetConnectAttr(hdl_dbconn_, SQL_ATTR_AUTOCOMMIT,
            value, SQL_NTS))) {
      ODBCContext::extract_error(SQL_HANDLE_DBC, hdl_dbconn_);
      return false;
  }
  return true;
}

bool ODBCLink::get_autocommit() const
{
  SQLINTEGER status;

  // toggle AUTOCOMMIT status
  if (!SQL_SUCCEEDED(
          SQLGetConnectAttr(hdl_dbconn_, SQL_ATTR_AUTOCOMMIT,
            &status, 0, nullptr))) {
      ODBCContext::extract_error(SQL_HANDLE_DBC, hdl_dbconn_);
  }
  return status != 0;
}

bool ODBCLink::commit()
{
  return end_transaction(true);
}

bool ODBCLink::rollback()
{
  return end_transaction(false);
}

bool ODBCLink::end_transaction(const bool is_commit)
{
  SQLSMALLINT type = SQL_COMMIT;

  if (!is_commit) {
    type = SQL_ROLLBACK;
  }

  // toggle AUTOCOMMIT status
  if (!SQL_SUCCEEDED(
          SQLEndTran(SQL_HANDLE_DBC, hdl_dbconn_, type))) {
      ODBCContext::extract_error(SQL_HANDLE_DBC, hdl_dbconn_);
      return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////


bool HHVM_FUNCTION(odbc_set_autocommit, const Resource& link, bool on_off)
{
  auto odbc_link = safe_cast<ODBCLink>(link);
  if (odbc_link == nullptr)
    return false;
  return odbc_link->set_autocommit(on_off);
}

bool HHVM_FUNCTION(odbc_get_autocommit, const Resource& link)
{
  auto odbc_link = safe_cast<ODBCLink>(link);
  if (odbc_link == nullptr)
    return false;
  return odbc_link->get_autocommit();
}

bool HHVM_FUNCTION(odbc_commit, const Resource& link)
{
  auto odbc_link = safe_cast<ODBCLink>(link);
  if (odbc_link == nullptr)
    return false;
  return odbc_link->commit();
}

Variant HHVM_FUNCTION(odbc_connect, const String& dsn, const String& username,
    const String& password, const Variant& cursor_type /* = 0 */)
{
  auto odbc_link = req::make<ODBCLink>();

  if (!odbc_link->connect(dsn, username, password)) {
    return false;
  }
  return Variant(std::move(odbc_link));
}

void HHVM_FUNCTION(odbc_close, const Resource& link)
{
  auto odbc_link = safe_cast<ODBCLink>(link);
  if (odbc_link != nullptr)
    odbc_link->close();
}

String HHVM_FUNCTION(odbc_error, const Variant& link)
{
  return String::FromCStr((char*)ODBCContext::get_last_error_code());
}

String HHVM_FUNCTION(odbc_errormsg, const Variant& link)
{
  return String::FromCStr((char*)ODBCContext::get_last_error_msg());
}

Variant HHVM_FUNCTION(odbc_exec, const Resource& link, const String& query,
                      const Variant& flags /* = 0 */)
{
  auto odbc_link = safe_cast<ODBCLink>(link);
  if (odbc_link == nullptr)
    return false;
  return odbc_link->exec(query);
}

bool HHVM_FUNCTION(odbc_execute, const Resource& result, const Variant& params)
{
  auto odbc_result = safe_cast<ODBCCursor>(result);
  if (odbc_result == nullptr)
    return false;
  return odbc_result->exec_prepared_query(params.toArray());
}

Variant HHVM_FUNCTION(odbc_fetch_array, const Resource& cursor,
                      const Variant& rownumber /* = 0 */)
{
  auto odbc_cursor = safe_cast<ODBCCursor>(cursor);
  if (odbc_cursor == nullptr)
    return false;
  return odbc_cursor->fetch();
}

int64_t HHVM_FUNCTION(odbc_num_rows, const Resource& cursor)
{
  auto odbc_cursor = safe_cast<ODBCCursor>(cursor);
  if (odbc_cursor == nullptr)
    return -1;
  return odbc_cursor->num_rows();
}

Variant HHVM_FUNCTION(odbc_prepare, const Resource& link, const String& query)
{
  auto odbc_link = safe_cast<ODBCLink>(link);
  if (odbc_link == nullptr)
    return init_null();
  return odbc_link->prepare(query);
}

bool HHVM_FUNCTION(odbc_rollback, const Resource& link)
{
  auto odbc_link = safe_cast<ODBCLink>(link);
  if (odbc_link == nullptr)
    return false;
  return odbc_link->rollback();
}

///////////////////////////////////////////////////////////////////////////////
static class ODBCExtension final : public Extension {
 public:
  ODBCExtension() : Extension("odbc") { }
  void moduleInit() override {
    HHVM_FE(odbc_set_autocommit);
    HHVM_FE(odbc_get_autocommit);
    HHVM_FE(odbc_commit);
    HHVM_FE(odbc_connect);
    HHVM_FE(odbc_close);
    HHVM_FE(odbc_error);
    HHVM_FE(odbc_errormsg);
    HHVM_FE(odbc_exec);
    HHVM_FE(odbc_execute);
    HHVM_FE(odbc_fetch_array);
    HHVM_FE(odbc_num_rows);
    HHVM_FE(odbc_prepare);
    HHVM_FE(odbc_rollback);

    loadSystemlib();
  }
} s_odbc_extension;

}

#endif // HAVE_UODBC
