/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/ext/ext_pdo.h>
#include <runtime/ext/pdo_driver.h>
#include <runtime/ext/pdo_mysql.h>
#include <runtime/ext/ext_class.h>
#include <runtime/ext/ext_function.h>
#include <runtime/ext/ext_stream.h>
#include <runtime/base/class_info.h>
#include <runtime/base/ini_setting.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/util/request_local.h>

#define PDO_HANDLE_DBH_ERR(dbh)                         \
  if (strcmp(dbh->error_code, PDO_ERR_NONE)) {          \
    pdo_handle_error(dbh, NULL);                        \
  }                                                     \

#define PDO_HANDLE_STMT_ERR(stmt)                       \
  if (strcmp(stmt->error_code, PDO_ERR_NONE)) {         \
    pdo_handle_error(stmt->dbh, stmt);                  \
  }                                                     \

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// PDO constants

const int64 q_pdo_PARAM_BOOL               = PDO_PARAM_BOOL;
const int64 q_pdo_PARAM_NULL               = PDO_PARAM_NULL;
const int64 q_pdo_PARAM_INT                = PDO_PARAM_INT;
const int64 q_pdo_PARAM_STR                = PDO_PARAM_STR;
const int64 q_pdo_PARAM_LOB                = PDO_PARAM_LOB;
const int64 q_pdo_PARAM_STMT               = PDO_PARAM_STMT;
const int64 q_pdo_PARAM_INPUT_OUTPUT       = PDO_PARAM_INPUT_OUTPUT;

const int64 q_pdo_PARAM_EVT_ALLOC          = PDO_PARAM_EVT_ALLOC;
const int64 q_pdo_PARAM_EVT_FREE           = PDO_PARAM_EVT_FREE;
const int64 q_pdo_PARAM_EVT_EXEC_PRE       = PDO_PARAM_EVT_EXEC_PRE;
const int64 q_pdo_PARAM_EVT_EXEC_POST      = PDO_PARAM_EVT_EXEC_POST;
const int64 q_pdo_PARAM_EVT_FETCH_PRE      = PDO_PARAM_EVT_FETCH_PRE;
const int64 q_pdo_PARAM_EVT_FETCH_POST     = PDO_PARAM_EVT_FETCH_POST;
const int64 q_pdo_PARAM_EVT_NORMALIZE      = PDO_PARAM_EVT_NORMALIZE;

const int64 q_pdo_FETCH_USE_DEFAULT        = PDO_FETCH_USE_DEFAULT;
const int64 q_pdo_FETCH_LAZY               = PDO_FETCH_LAZY;
const int64 q_pdo_FETCH_ASSOC              = PDO_FETCH_ASSOC;
const int64 q_pdo_FETCH_NUM                = PDO_FETCH_NUM;
const int64 q_pdo_FETCH_BOTH               = PDO_FETCH_BOTH;
const int64 q_pdo_FETCH_OBJ                = PDO_FETCH_OBJ;
const int64 q_pdo_FETCH_BOUND              = PDO_FETCH_BOUND;
const int64 q_pdo_FETCH_COLUMN             = PDO_FETCH_COLUMN;
const int64 q_pdo_FETCH_CLASS              = PDO_FETCH_CLASS;
const int64 q_pdo_FETCH_INTO               = PDO_FETCH_INTO;
const int64 q_pdo_FETCH_FUNC               = PDO_FETCH_FUNC;
const int64 q_pdo_FETCH_GROUP              = PDO_FETCH_GROUP;
const int64 q_pdo_FETCH_UNIQUE             = PDO_FETCH_UNIQUE;
const int64 q_pdo_FETCH_KEY_PAIR           = PDO_FETCH_KEY_PAIR;
const int64 q_pdo_FETCH_CLASSTYPE          = PDO_FETCH_CLASSTYPE;
const int64 q_pdo_FETCH_SERIALIZE          = PDO_FETCH_SERIALIZE;
const int64 q_pdo_FETCH_PROPS_LATE         = PDO_FETCH_PROPS_LATE;
const int64 q_pdo_FETCH_NAMED              = PDO_FETCH_NAMED;

const int64 q_pdo_ATTR_AUTOCOMMIT          = PDO_ATTR_AUTOCOMMIT;
const int64 q_pdo_ATTR_PREFETCH            = PDO_ATTR_PREFETCH;
const int64 q_pdo_ATTR_TIMEOUT             = PDO_ATTR_TIMEOUT;
const int64 q_pdo_ATTR_ERRMODE             = PDO_ATTR_ERRMODE;
const int64 q_pdo_ATTR_SERVER_VERSION      = PDO_ATTR_SERVER_VERSION;
const int64 q_pdo_ATTR_CLIENT_VERSION      = PDO_ATTR_CLIENT_VERSION;
const int64 q_pdo_ATTR_SERVER_INFO         = PDO_ATTR_SERVER_INFO;
const int64 q_pdo_ATTR_CONNECTION_STATUS   = PDO_ATTR_CONNECTION_STATUS;
const int64 q_pdo_ATTR_CASE                = PDO_ATTR_CASE;
const int64 q_pdo_ATTR_CURSOR_NAME         = PDO_ATTR_CURSOR_NAME;
const int64 q_pdo_ATTR_CURSOR              = PDO_ATTR_CURSOR;
const int64 q_pdo_ATTR_ORACLE_NULLS        = PDO_ATTR_ORACLE_NULLS;
const int64 q_pdo_ATTR_PERSISTENT          = PDO_ATTR_PERSISTENT;
const int64 q_pdo_ATTR_STATEMENT_CLASS     = PDO_ATTR_STATEMENT_CLASS;
const int64 q_pdo_ATTR_FETCH_TABLE_NAMES   = PDO_ATTR_FETCH_TABLE_NAMES;
const int64 q_pdo_ATTR_FETCH_CATALOG_NAMES = PDO_ATTR_FETCH_CATALOG_NAMES;
const int64 q_pdo_ATTR_DRIVER_NAME         = PDO_ATTR_DRIVER_NAME;
const int64 q_pdo_ATTR_STRINGIFY_FETCHES   = PDO_ATTR_STRINGIFY_FETCHES;
const int64 q_pdo_ATTR_MAX_COLUMN_LEN      = PDO_ATTR_MAX_COLUMN_LEN;
const int64 q_pdo_ATTR_EMULATE_PREPARES    = PDO_ATTR_EMULATE_PREPARES;
const int64 q_pdo_ATTR_DEFAULT_FETCH_MODE  = PDO_ATTR_DEFAULT_FETCH_MODE;

const int64 q_pdo_ERRMODE_SILENT           = PDO_ERRMODE_SILENT;
const int64 q_pdo_ERRMODE_WARNING          = PDO_ERRMODE_WARNING;
const int64 q_pdo_ERRMODE_EXCEPTION        = PDO_ERRMODE_EXCEPTION;

const int64 q_pdo_CASE_NATURAL             = PDO_CASE_NATURAL;
const int64 q_pdo_CASE_LOWER               = PDO_CASE_LOWER;
const int64 q_pdo_CASE_UPPER               = PDO_CASE_UPPER;

const int64 q_pdo_NULL_NATURAL             = PDO_NULL_NATURAL;
const int64 q_pdo_NULL_EMPTY_STRING        = PDO_NULL_EMPTY_STRING;
const int64 q_pdo_NULL_TO_STRING           = PDO_NULL_TO_STRING;

const StaticString q_pdo_ERR_NONE          = PDO_ERR_NONE;

const int64 q_pdo_FETCH_ORI_NEXT           = PDO_FETCH_ORI_NEXT;
const int64 q_pdo_FETCH_ORI_PRIOR          = PDO_FETCH_ORI_PRIOR;
const int64 q_pdo_FETCH_ORI_FIRST          = PDO_FETCH_ORI_FIRST;
const int64 q_pdo_FETCH_ORI_LAST           = PDO_FETCH_ORI_LAST;
const int64 q_pdo_FETCH_ORI_ABS            = PDO_FETCH_ORI_ABS;
const int64 q_pdo_FETCH_ORI_REL            = PDO_FETCH_ORI_REL;

const int64 q_pdo_CURSOR_FWDONLY           = PDO_CURSOR_FWDONLY;
const int64 q_pdo_CURSOR_SCROLL            = PDO_CURSOR_SCROLL;

///////////////////////////////////////////////////////////////////////////////

const int64 q_pdo_MYSQL_ATTR_USE_BUFFERED_QUERY =
  PDO_MYSQL_ATTR_USE_BUFFERED_QUERY;
const int64 q_pdo_MYSQL_ATTR_LOCAL_INFILE = PDO_MYSQL_ATTR_LOCAL_INFILE;
const int64 q_pdo_MYSQL_ATTR_MAX_BUFFER_SIZE =
  PDO_MYSQL_ATTR_MAX_BUFFER_SIZE;
const int64 q_pdo_MYSQL_ATTR_INIT_COMMAND = PDO_MYSQL_ATTR_INIT_COMMAND;
const int64 q_pdo_MYSQL_ATTR_READ_DEFAULT_FILE =
  PDO_MYSQL_ATTR_READ_DEFAULT_FILE;
const int64 q_pdo_MYSQL_ATTR_READ_DEFAULT_GROUP =
  PDO_MYSQL_ATTR_READ_DEFAULT_GROUP;
const int64 q_pdo_MYSQL_ATTR_COMPRESS     = PDO_MYSQL_ATTR_COMPRESS;
const int64 q_pdo_MYSQL_ATTR_DIRECT_QUERY = PDO_MYSQL_ATTR_DIRECT_QUERY;
const int64 q_pdo_MYSQL_ATTR_FOUND_ROWS   = PDO_MYSQL_ATTR_FOUND_ROWS;
const int64 q_pdo_MYSQL_ATTR_IGNORE_SPACE = PDO_MYSQL_ATTR_IGNORE_SPACE;

///////////////////////////////////////////////////////////////////////////////
// extension functions

Array f_pdo_drivers() {
  Array ret = Array::Create();
  const PDODriverMap &drivers = PDODriver::GetDrivers();
  for (PDODriverMap::const_iterator iter = drivers.begin();
       iter != drivers.end(); ++iter) {
    ret.append(iter->second->getName());
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// error handling

struct pdo_sqlstate_info {
  const char *state;
  const char *desc;
};

static const struct pdo_sqlstate_info err_initializer[] = {
  { "00000",  "No error" },
  { "01000",  "Warning" },
  { "01001",  "Cursor operation conflict" },
  { "01002",  "Disconnect error" },
  { "01003",  "NULL value eliminated in set function" },
  { "01004",  "String data, right truncated" },
  { "01006",  "Privilege not revoked" },
  { "01007",  "Privilege not granted" },
  { "01008",  "Implicit zero bit padding" },
  { "0100C",  "Dynamic result sets returned" },
  { "01P01",  "Deprecated feature" },
  { "01S00",  "Invalid connection string attribute" },
  { "01S01",  "Error in row" },
  { "01S02",  "Option value changed" },
  { "01S06",
    "Attempt to fetch before the result set returned the first rowset" },
  { "01S07",  "Fractional truncation" },
  { "01S08",  "Error saving File DSN" },
  { "01S09",  "Invalid keyword" },
  { "02000",  "No data" },
  { "02001",  "No additional dynamic result sets returned" },
  { "03000",  "Sql statement not yet complete" },
  { "07002",  "COUNT field incorrect" },
  { "07005",  "Prepared statement not a cursor-specification" },
  { "07006",  "Restricted data type attribute violation" },
  { "07009",  "Invalid descriptor index" },
  { "07S01",  "Invalid use of default parameter" },
  { "08000",  "Connection exception" },
  { "08001",  "Client unable to establish connection" },
  { "08002",  "Connection name in use" },
  { "08003",  "Connection does not exist" },
  { "08004",  "Server rejected the connection" },
  { "08006",  "Connection failure" },
  { "08007",  "Connection failure during transaction" },
  { "08S01",  "Communication link failure" },
  { "09000",  "Triggered action exception" },
  { "0A000",  "Feature not supported" },
  { "0B000",  "Invalid transaction initiation" },
  { "0F000",  "Locator exception" },
  { "0F001",  "Invalid locator specification" },
  { "0L000",  "Invalid grantor" },
  { "0LP01",  "Invalid grant operation" },
  { "0P000",  "Invalid role specification" },
  { "21000",  "Cardinality violation" },
  { "21S01",  "Insert value list does not match column list" },
  { "21S02",  "Degree of derived table does not match column list" },
  { "22000",  "Data exception" },
  { "22001",  "String data, right truncated" },
  { "22002",  "Indicator variable required but not supplied" },
  { "22003",  "Numeric value out of range" },
  { "22004",  "Null value not allowed" },
  { "22005",  "Error in assignment" },
  { "22007",  "Invalid datetime format" },
  { "22008",  "Datetime field overflow" },
  { "22009",  "Invalid time zone displacement value" },
  { "2200B",  "Escape character conflict" },
  { "2200C",  "Invalid use of escape character" },
  { "2200D",  "Invalid escape octet" },
  { "2200F",  "Zero length character string" },
  { "2200G",  "Most specific type mismatch" },
  { "22010",  "Invalid indicator parameter value" },
  { "22011",  "Substring error" },
  { "22012",  "Division by zero" },
  { "22015",  "Interval field overflow" },
  { "22018",  "Invalid character value for cast specification" },
  { "22019",  "Invalid escape character" },
  { "2201B",  "Invalid regular expression" },
  { "2201E",  "Invalid argument for logarithm" },
  { "2201F",  "Invalid argument for power function" },
  { "2201G",  "Invalid argument for width bucket function" },
  { "22020",  "Invalid limit value" },
  { "22021",  "Character not in repertoire" },
  { "22022",  "Indicator overflow" },
  { "22023",  "Invalid parameter value" },
  { "22024",  "Unterminated c string" },
  { "22025",  "Invalid escape sequence" },
  { "22026",  "String data, length mismatch" },
  { "22027",  "Trim error" },
  { "2202E",  "Array subscript error" },
  { "22P01",  "Floating point exception" },
  { "22P02",  "Invalid text representation" },
  { "22P03",  "Invalid binary representation" },
  { "22P04",  "Bad copy file format" },
  { "22P05",  "Untranslatable character" },
  { "23000",  "Integrity constraint violation" },
  { "23001",  "Restrict violation" },
  { "23502",  "Not null violation" },
  { "23503",  "Foreign key violation" },
  { "23505",  "Unique violation" },
  { "23514",  "Check violation" },
  { "24000",  "Invalid cursor state" },
  { "25000",  "Invalid transaction state" },
  { "25001",  "Active sql transaction" },
  { "25002",  "Branch transaction already active" },
  { "25003",  "Inappropriate access mode for branch transaction" },
  { "25004",  "Inappropriate isolation level for branch transaction" },
  { "25005",  "No active sql transaction for branch transaction" },
  { "25006",  "Read only sql transaction" },
  { "25007",  "Schema and data statement mixing not supported" },
  { "25008",  "Held cursor requires same isolation level" },
  { "25P01",  "No active sql transaction" },
  { "25P02",  "In failed sql transaction" },
  { "25S01",  "Transaction state" },
  { "25S02",  "Transaction is still active" },
  { "25S03",  "Transaction is rolled back" },
  { "26000",  "Invalid sql statement name" },
  { "27000",  "Triggered data change violation" },
  { "28000",  "Invalid authorization specification" },
  { "2B000",  "Dependent privilege descriptors still exist" },
  { "2BP01",  "Dependent objects still exist" },
  { "2D000",  "Invalid transaction termination" },
  { "2F000",  "Sql routine exception" },
  { "2F002",  "Modifying sql data not permitted" },
  { "2F003",  "Prohibited sql statement attempted" },
  { "2F004",  "Reading sql data not permitted" },
  { "2F005",  "Function executed no return statement" },
  { "34000",  "Invalid cursor name" },
  { "38000",  "External routine exception" },
  { "38001",  "Containing sql not permitted" },
  { "38002",  "Modifying sql data not permitted" },
  { "38003",  "Prohibited sql statement attempted" },
  { "38004",  "Reading sql data not permitted" },
  { "39000",  "External routine invocation exception" },
  { "39001",  "Invalid sqlstate returned" },
  { "39004",  "Null value not allowed" },
  { "39P01",  "Trigger protocol violated" },
  { "39P02",  "Srf protocol violated" },
  { "3B000",  "Savepoint exception" },
  { "3B001",  "Invalid savepoint specification" },
  { "3C000",  "Duplicate cursor name" },
  { "3D000",  "Invalid catalog name" },
  { "3F000",  "Invalid schema name" },
  { "40000",  "Transaction rollback" },
  { "40001",  "Serialization failure" },
  { "40002",  "Transaction integrity constraint violation" },
  { "40003",  "Statement completion unknown" },
  { "40P01",  "Deadlock detected" },
  { "42000",  "Syntax error or access violation" },
  { "42501",  "Insufficient privilege" },
  { "42601",  "Syntax error" },
  { "42602",  "Invalid name" },
  { "42611",  "Invalid column definition" },
  { "42622",  "Name too long" },
  { "42701",  "Duplicate column" },
  { "42702",  "Ambiguous column" },
  { "42703",  "Undefined column" },
  { "42704",  "Undefined object" },
  { "42710",  "Duplicate object" },
  { "42712",  "Duplicate alias" },
  { "42723",  "Duplicate function" },
  { "42725",  "Ambiguous function" },
  { "42803",  "Grouping error" },
  { "42804",  "Datatype mismatch" },
  { "42809",  "Wrong object type" },
  { "42830",  "Invalid foreign key" },
  { "42846",  "Cannot coerce" },
  { "42883",  "Undefined function" },
  { "42939",  "Reserved name" },
  { "42P01",  "Undefined table" },
  { "42P02",  "Undefined parameter" },
  { "42P03",  "Duplicate cursor" },
  { "42P04",  "Duplicate database" },
  { "42P05",  "Duplicate prepared statement" },
  { "42P06",  "Duplicate schema" },
  { "42P07",  "Duplicate table" },
  { "42P08",  "Ambiguous parameter" },
  { "42P09",  "Ambiguous alias" },
  { "42P10",  "Invalid column reference" },
  { "42P11",  "Invalid cursor definition" },
  { "42P12",  "Invalid database definition" },
  { "42P13",  "Invalid function definition" },
  { "42P14",  "Invalid prepared statement definition" },
  { "42P15",  "Invalid schema definition" },
  { "42P16",  "Invalid table definition" },
  { "42P17",  "Invalid object definition" },
  { "42P18",  "Indeterminate datatype" },
  { "42S01",  "Base table or view already exists" },
  { "42S02",  "Base table or view not found" },
  { "42S11",  "Index already exists" },
  { "42S12",  "Index not found" },
  { "42S21",  "Column already exists" },
  { "42S22",  "Column not found" },
  { "44000",  "WITH CHECK OPTION violation" },
  { "53000",  "Insufficient resources" },
  { "53100",  "Disk full" },
  { "53200",  "Out of memory" },
  { "53300",  "Too many connections" },
  { "54000",  "Program limit exceeded" },
  { "54001",  "Statement too complex" },
  { "54011",  "Too many columns" },
  { "54023",  "Too many arguments" },
  { "55000",  "Object not in prerequisite state" },
  { "55006",  "Object in use" },
  { "55P02",  "Cant change runtime param" },
  { "55P03",  "Lock not available" },
  { "57000",  "Operator intervention" },
  { "57014",  "Query canceled" },
  { "57P01",  "Admin shutdown" },
  { "57P02",  "Crash shutdown" },
  { "57P03",  "Cannot connect now" },
  { "58030",  "Io error" },
  { "58P01",  "Undefined file" },
  { "58P02",  "Duplicate file" },
  { "F0000",  "Config file error" },
  { "F0001",  "Lock file exists" },
  { "HY000",  "General error" },
  { "HY001",  "Memory allocation error" },
  { "HY003",  "Invalid application buffer type" },
  { "HY004",  "Invalid SQL data type" },
  { "HY007",  "Associated statement is not prepared" },
  { "HY008",  "Operation canceled" },
  { "HY009",  "Invalid use of null pointer" },
  { "HY010",  "Function sequence error" },
  { "HY011",  "Attribute cannot be set now" },
  { "HY012",  "Invalid transaction operation code" },
  { "HY013",  "Memory management error" },
  { "HY014",  "Limit on the number of handles exceeded" },
  { "HY015",  "No cursor name available" },
  { "HY016",  "Cannot modify an implementation row descriptor" },
  { "HY017",  "Invalid use of an automatically allocated descriptor handle" },
  { "HY018",  "Server declined cancel request" },
  { "HY019",  "Non-character and non-binary data sent in pieces" },
  { "HY020",  "Attempt to concatenate a null value" },
  { "HY021",  "Inconsistent descriptor information" },
  { "HY024",  "Invalid attribute value" },
  { "HY090",  "Invalid string or buffer length" },
  { "HY091",  "Invalid descriptor field identifier" },
  { "HY092",  "Invalid attribute/option identifier" },
  { "HY093",  "Invalid parameter number" },
  { "HY095",  "Function type out of range" },
  { "HY096",  "Invalid information type" },
  { "HY097",  "Column type out of range" },
  { "HY098",  "Scope type out of range" },
  { "HY099",  "Nullable type out of range" },
  { "HY100",  "Uniqueness option type out of range" },
  { "HY101",  "Accuracy option type out of range" },
  { "HY103",  "Invalid retrieval code" },
  { "HY104",  "Invalid precision or scale value" },
  { "HY105",  "Invalid parameter type" },
  { "HY106",  "Fetch type out of range" },
  { "HY107",  "Row value out of range" },
  { "HY109",  "Invalid cursor position" },
  { "HY110",  "Invalid driver completion" },
  { "HY111",  "Invalid bookmark value" },
  { "HYC00",  "Optional feature not implemented" },
  { "HYT00",  "Timeout expired" },
  { "HYT01",  "Connection timeout expired" },
  { "IM001",  "Driver does not support this function" },
  { "IM002",  "Data source name not found and no default driver specified" },
  { "IM003",  "Specified driver could not be loaded" },
  { "IM004",  "Driver's SQLAllocHandle on SQL_HANDLE_ENV failed" },
  { "IM005",  "Driver's SQLAllocHandle on SQL_HANDLE_DBC failed" },
  { "IM006",  "Driver's SQLSetConnectAttr failed" },
  { "IM007",  "No data source or driver specified; dialog prohibited" },
  { "IM008",  "Dialog failed" },
  { "IM009",  "Unable to load translation DLL" },
  { "IM010",  "Data source name too long" },
  { "IM011",  "Driver name too long" },
  { "IM012",  "DRIVER keyword syntax error" },
  { "IM013",  "Trace file error" },
  { "IM014",  "Invalid name of File DSN" },
  { "IM015",  "Corrupt file data source" },
  { "P0000",  "Plpgsql error" },
  { "P0001",  "Raise exception" },
  { "XX000",  "Internal error" },
  { "XX001",  "Data corrupted" },
  { "XX002",  "Index corrupted" }
};

class PDOErrorHash : private hphp_const_char_map<const char *> {
public:
  PDOErrorHash() {
    for (unsigned int i = 0;
         i < sizeof(err_initializer)/sizeof(err_initializer[0]); i++) {
      const struct pdo_sqlstate_info *info = &err_initializer[i];
      (*this)[info->state] = info->desc;
    }
  }

  const char *description(const char *state) {
    const_iterator iter = find(state);
    if (iter != end()) {
      return iter->second;
    }
    return "<<Unknown error>>";
  }
};
static PDOErrorHash s_err_hash;

void throw_pdo_exception(CVarRef code, CVarRef info, const char *fmt, ...) {
  c_pdoexception *e = NEW(c_pdoexception)();
  e->m_code = code;

  va_list ap;
  va_start(ap, fmt);
  char buf[20480];
  vsnprintf(buf, sizeof(buf), fmt, ap);
  e->m_message = String(buf, CopyString);
  va_end(ap);

  if (!info.isNull()) {
    e->set("errorInfo", info);
  }
  throw Object(e);
}

void pdo_raise_impl_error(sp_PDOConnection dbh, sp_PDOStatement stmt,
                          const char *sqlstate, const char *supp) {
  PDOErrorType *pdo_err = &dbh->error_code;
  if (stmt.get()) {
    pdo_err = &stmt->error_code;
  }
  strcpy(*pdo_err, sqlstate);

  const char *msg = s_err_hash.description(sqlstate);
  string err = "SQLSTATE["; err += sqlstate; err += "]: "; err += msg;
  if (supp) {
    err += ": "; err += supp;
  }

  if (dbh->error_mode != PDO_ERRMODE_EXCEPTION) {
    raise_warning("%s", err.c_str());
  } else {
    Array info;
    info.append(String(*pdo_err, CopyString));
    info.append(0LL);
    throw_pdo_exception(String(sqlstate, CopyString), info, "%s", err.c_str());
  }
}

static void pdo_handle_error(sp_PDOConnection dbh, sp_PDOStatement stmt) {
  if (dbh->error_mode == PDO_ERRMODE_SILENT) {
    return;
  }
  PDOErrorType *pdo_err = &dbh->error_code;
  if (stmt.get()) {
    pdo_err = &stmt->error_code;
  }

  /* hash sqlstate to error messages */
  const char *msg = s_err_hash.description(*pdo_err);

  int64 native_code = 0;
  String supp;
  Array info;
  if (dbh->support(PDOConnection::MethodFetchErr)) {
    info = Array::Create();
    info.append(String(*pdo_err, CopyString));
    if (dbh->fetchErr(stmt.get(), info)) {
      if (info.exists(1)) {
        native_code = info[1].toInt64();
      }
      if (info.exists(2)) {
        supp = info[2].toString();
      }
    }
  }

  string err = "SQLSTATE["; err += *pdo_err; err += "]: "; err += msg;
  if (!supp.empty()) {
    err += ": "; err += String(native_code).data();
    err += " ";  err += supp.data();
  }

  if (dbh->error_mode != PDO_ERRMODE_EXCEPTION) {
    raise_warning("%s", err.c_str());
  } else {
    throw_pdo_exception(String(*pdo_err, CopyString), info, "%s", err.c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////
// helpers for PDO class

static inline int64 pdo_attr_lval(CArrRef options, PDOAttributeType name,
                                  int64 defval) {
  if (options.exists(name)) {
    return options[name].toInt64();
  }
  return defval;
}

static inline String pdo_attr_strval(CArrRef options, PDOAttributeType name,
                                     CStrRef defval) {
  if (options.exists(name)) {
    return options[name].toString();
  }
  return defval;
}

static Object pdo_stmt_instantiate(sp_PDOConnection dbh, CStrRef clsname,
                                   CVarRef ctor_args) {
  String name = clsname;
  if (name.empty()) {
    name = "PDOStatement";
  }
  if (!ctor_args.isNull() && !ctor_args.isArray()) {
    pdo_raise_impl_error(dbh, NULL, "HY000",
                         "constructor arguments must be passed as an array");
    return Object();
  }
  return create_object(name, Array(), false);
}

static void pdo_stmt_construct(sp_PDOStatement stmt, Object object,
                               CStrRef clsname, CVarRef ctor_args) {
  const ClassInfo *cls = ClassInfo::FindClass(clsname.data());
  if (cls) {
    const char *constructor = cls->getConstructor();
    if (constructor) {
      object->set("queryString", stmt->query_string);
      object->o_invoke(constructor, ctor_args, -1);
    }
  }
}

static bool valid_statement_class(sp_PDOConnection dbh, CVarRef opt,
                                  String &clsname, Variant &ctor_args) {
  if (!opt.isArray() || !opt.toArray().exists(0) || !opt[0].isString() ||
      !f_class_exists(opt[0])) {
    pdo_raise_impl_error
      (dbh, NULL, "HY000",
       "PDO::ATTR_STATEMENT_CLASS requires format array(classname, "
       "array(ctor_args)); the classname must be a string specifying "
       "an existing class");
    PDO_HANDLE_DBH_ERR(dbh);
    return false;
  }
  clsname = opt[0].toString();
  if (!f_is_subclass_of(clsname, "PDOStatement")) {
    pdo_raise_impl_error
      (dbh, NULL, "HY000",
       "user-supplied statement class must be derived from PDOStatement");
    PDO_HANDLE_DBH_ERR(dbh);
    return false;
  }
  const ClassInfo *cls = ClassInfo::FindClass(clsname.data());
  if (cls) {
    ClassInfo::MethodInfo *method = cls->getMethodInfo("__construct");
    if (!method) {
      method = cls->getMethodInfo(clsname.data());
    }
    if (method && (method->attribute & ClassInfo::IsPublic)) {
      pdo_raise_impl_error
        (dbh, NULL, "HY000",
         "user-supplied statement class cannot have a public constructor");
      PDO_HANDLE_DBH_ERR(dbh);
      return false;
    }
  }
  if (opt.toArray().exists(1)) {
    Variant item = opt[1];
    if (!item.isArray()) {
      pdo_raise_impl_error
        (dbh, NULL, "HY000",
         "PDO::ATTR_STATEMENT_CLASS requires format array(classname, "
         "ctor_args); ctor_args must be an array");
      PDO_HANDLE_DBH_ERR(dbh);
      return false;
    }
    ctor_args = item;
  }
  return true;
}

static bool pdo_stmt_describe_columns(sp_PDOStatement stmt) {
  for (int col = 0; col < stmt->column_count; col++) {
    if (!stmt->describer(col)) {
      return false;
    }

    String &name = stmt->columns[col].toObject().getTyped<PDOColumn>()->name;

    /* if we are applying case conversions on column names, do so now */
    if (stmt->dbh->native_case != stmt->dbh->desired_case &&
        stmt->dbh->desired_case != PDO_CASE_NATURAL) {
      switch (stmt->dbh->desired_case) {
      case PDO_CASE_UPPER:
        name = StringUtil::ToUpper(name);
        break;
      case PDO_CASE_LOWER:
        name = StringUtil::ToLower(name);
        break;
      default:;
      }
    }

    if (stmt->bound_columns.exists(name)) {
      PDOBoundParam *param =
        stmt->bound_columns[name].toObject().getTyped<PDOBoundParam>();
      param->paramno = col;
    }
  }
  return true;
}

static bool pdo_stmt_verify_mode(sp_PDOStatement stmt, int64 mode,
                                 bool fetch_all) {
  int flags = mode & PDO_FETCH_FLAGS;
  mode = mode & ~PDO_FETCH_FLAGS;

  if (mode < 0 || mode > PDO_FETCH__MAX) {
    pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "invalid fetch mode");
    return false;
  }

  if (mode == PDO_FETCH_USE_DEFAULT) {
    flags = stmt->default_fetch_type & PDO_FETCH_FLAGS;
    mode = stmt->default_fetch_type & ~PDO_FETCH_FLAGS;
  }

  switch (mode) {
  case PDO_FETCH_FUNC:
    if (!fetch_all) {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                           "PDO::FETCH_FUNC is only allowed in "
                           "PDOStatement::fetchAll()");
      return false;
    }
    return true;

  case PDO_FETCH_LAZY:
    if (fetch_all) {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                           "PDO::FETCH_LAZY can't be used with "
                           "PDOStatement::fetchAll()");
      return false;
    }

  default:
    if ((flags & PDO_FETCH_SERIALIZE) == PDO_FETCH_SERIALIZE) {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                           "PDO::FETCH_SERIALIZE can only be used "
                           "together with PDO::FETCH_CLASS");
      return false;
    }
    if ((flags & PDO_FETCH_CLASSTYPE) == PDO_FETCH_CLASSTYPE) {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                           "PDO::FETCH_CLASSTYPE can only be used "
                           "together with PDO::FETCH_CLASS");
      return false;
    }
    if (mode >= PDO_FETCH__MAX) {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "invalid fetch mode");
      return false;
    }
    /* no break; */

  case PDO_FETCH_CLASS:
    break;
  }
  return true;
}

static bool do_fetch_class_prepare(sp_PDOStatement stmt) {
  String clsname = stmt->fetch.clsname;
  if (clsname.empty()) {
    stmt->fetch.clsname = "stdclass";
  }
  stmt->fetch.constructor = NULL;
  const ClassInfo *cls = ClassInfo::FindClass(clsname.data());
  if (cls) {
    const char *constructor = cls->getConstructor();
    if (constructor) {
      stmt->fetch.constructor = constructor;
      return true;
    }
  }
  if (!stmt->fetch.ctor_args.isNull()) {
    pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                         "user-supplied class does not have a constructor, "
                         "use NULL for the ctor_params parameter, or simply "
                         "omit it");
    return false;
  }
  return true; /* no ctor no args is also ok */
}

static bool pdo_stmt_set_fetch_mode(sp_PDOStatement stmt, int _argc, int64 mode,
                                    CArrRef _argv) {
  _argc = _argv.size() + 1;

  if (stmt->default_fetch_type == PDO_FETCH_INTO) {
    stmt->fetch.into.reset();
  }
  stmt->default_fetch_type = PDO_FETCH_BOTH;

  if (!pdo_stmt_verify_mode(stmt, mode, false)) {
    strcpy(stmt->error_code, PDO_ERR_NONE);
    return false;
  }

  int flags = mode & PDO_FETCH_FLAGS;
  bool retval = false;
  switch (mode & ~PDO_FETCH_FLAGS) {
  case PDO_FETCH_USE_DEFAULT:
  case PDO_FETCH_LAZY:
  case PDO_FETCH_ASSOC:
  case PDO_FETCH_NUM:
  case PDO_FETCH_BOTH:
  case PDO_FETCH_OBJ:
  case PDO_FETCH_BOUND:
  case PDO_FETCH_NAMED:
  case PDO_FETCH_KEY_PAIR:
    if (_argc != 1) {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                           "fetch mode doesn't allow any extra arguments");
    } else {
      retval = true;
    }
    break;

  case PDO_FETCH_COLUMN:
    if (_argc != 2) {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                           "fetch mode requires the colno argument");
    } else  if (!_argv[0].isInteger()) {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                           "colno must be an integer");
    } else {
      stmt->fetch.column = _argv[0].toInt64();
      retval = true;
    }
    break;

  case PDO_FETCH_CLASS:
    /* Gets its class name from 1st column */
    if ((flags & PDO_FETCH_CLASSTYPE) == PDO_FETCH_CLASSTYPE) {
      if (_argc != 1) {
        pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                             "fetch mode doesn't allow any extra arguments");
      } else {
        stmt->fetch.clsname.clear();
        retval = true;
      }
    } else {
      if (_argc < 2) {
        pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                             "fetch mode requires the classname argument");
      } else if (_argc > 3) {
        pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                             "too many arguments");
      } else if (!_argv[0].isString()) {
        pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                             "classname must be a string");
      } else {
        retval = f_class_exists(_argv[0]);
        if (retval) {
          stmt->fetch.clsname = _argv[0].toString();
        }
      }
    }

    if (retval) {
      stmt->fetch.ctor_args.reset();
      if (_argc == 3) {
        if (!_argv[1].isNull() && !_argv[1].isArray()) {
          pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                               "ctor_args must be either NULL or an array");
          retval = false;
        } else {
          stmt->fetch.ctor_args = _argv[1];
        }
      }

      if (retval) {
        do_fetch_class_prepare(stmt);
      }
    }

    break;

  case PDO_FETCH_INTO:
    if (_argc != 2) {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                           "fetch mode requires the object parameter");
    } else if (!_argv[0].isObject()) {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                           "object must be an object");
    } else {
      retval = true;
    }

    if (retval) {
      stmt->fetch.into = _argv[0];
    }
    break;

  default:
    pdo_raise_impl_error(stmt->dbh, stmt, "22003",
                         "Invalid fetch mode specified");
  }

  if (retval) {
    stmt->default_fetch_type = (PDOFetchType)mode;
  }

  /*
   * PDO error (if any) has already been raised at this point.
   *
   * The error_code is cleared, otherwise the caller will read the
   * last error message from the driver.
   *
   */
  strcpy(stmt->error_code, PDO_ERR_NONE);
  return retval;
}

///////////////////////////////////////////////////////////////////////////////

class PDORequestData : public RequestEventHandler {
public:
  virtual void requestInit() {
  }

  virtual void requestShutdown() {
    for (set<PDOConnection*>::iterator iter = m_persistent_connections.begin();
         iter != m_persistent_connections.end(); ++iter) {
      (*iter)->persistentSave();
    }
  }

public:
  set<PDOConnection*> m_persistent_connections;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(PDORequestData, s_pdo_request_data);

///////////////////////////////////////////////////////////////////////////////
// PDO

c_pdo::c_pdo() {
}

c_pdo::~c_pdo() {
  m_dbh.reset(); // needed for sweeping
}

void c_pdo::t___construct(CStrRef dsn, CStrRef username /* = null_string */,
                          CStrRef password /* = null_string */,
                          CArrRef options /* = null_array */) {
  String data_source = dsn;

  /* parse the data source name */
  const char *colon = strchr(data_source.data(), ':');
  if (!colon) {
    /* let's see if this string has a matching dsn in the php.ini */
    String name = "pdo.dsn."; name += data_source;
    String ini_dsn;
    if (!IniSetting::Get(name, ini_dsn)) {
      throw_pdo_exception(null, null, "invalid data source name");
    }
    data_source = ini_dsn;
    colon = strchr(data_source.data(), ':');
    if (!colon) {
      throw_pdo_exception(null, null, "invalid data source name (via INI: %s)",
                          ini_dsn.data());
    }
  }

  if (!strncmp(data_source.data(), "uri:", 4)) {
    /* the specified URI holds connection details */
    Variant stream = File::Open(data_source.substr(4), "rb");
    if (same(stream, false)) {
      throw_pdo_exception(null, null, "invalid data source URI");
    }
    data_source = stream.toObject().getTyped<File>()->readLine(1024);
    colon = strchr(data_source.data(), ':');
    if (!colon) {
      throw_pdo_exception(null, null, "invalid data source name (via URI)");
    }
  }

  const PDODriverMap &drivers = PDODriver::GetDrivers();
  String name = data_source.substr(0, colon - data_source.data());
  PDODriverMap::const_iterator iter = drivers.find(name.data());
  if (iter == drivers.end()) {
    /* NB: don't want to include the data_source in the error message as
     * it might contain a password */
    throw_pdo_exception(null, null, "could not find driver");
  }
  PDODriver *driver = iter->second;

  /* is this supposed to be a persistent connection ? */
  bool is_persistent = false;
  bool call_factory = true;
  String shashkey;
  if (!options.empty()) {
    StringBuffer hashkey;
    if (options.exists(PDO_ATTR_PERSISTENT)) {
      Variant v = options[PDO_ATTR_PERSISTENT];
      String sv = v.toString();
      if (v.isString() && !sv.isNumeric() && !sv.empty()) {
        /* user specified key */
        hashkey.printf("PDO:DBH:DSN=%s:%s:%s:%s",
                       data_source.data(), username.data(),
                       password.data(), sv.data());
        is_persistent = true;
      } else {
        is_persistent = v.toInt64();
        hashkey.printf("PDO:DBH:DSN=%s:%s:%s",
                       data_source.data(), username.data(),
                       password.data());
      }
    }

    if (is_persistent) {
      shashkey = hashkey.detach();
      /* let's see if we have one cached.... */
      m_dbh = dynamic_cast<PDOConnection*>
        (g_persistentObjects->get(PDOConnection::PersistentKey,
                                  shashkey.data()));
      m_dbh->persistentRestore();
      s_pdo_request_data->m_persistent_connections.insert(m_dbh.get());

      /* is the connection still alive ? */
      if (m_dbh->support(PDOConnection::MethodCheckLiveness) &&
          !m_dbh->checkLiveness()) {
        /* nope... need to kill it */
        m_dbh = NULL;
      }

      if (m_dbh.get()) {
        call_factory = false;
      } else {
        /* need a brand new pdbh */
        m_dbh = driver->createConnection(colon+1, username, password, options);
        if (m_dbh.get() == NULL) {
          throw_pdo_exception(null, null, "unable to create a connection");
        }
        m_dbh->persistent_id = string(shashkey.data(), shashkey.size());
      }
    }
  }
  if (!m_dbh.get()) {
    m_dbh = driver->createConnection(colon+1, username, password, options);
    if (m_dbh.get() == NULL) {
      throw_pdo_exception(null, null, "unable to create a connection");
    }
  }

  if (call_factory) {
    m_dbh->default_fetch_type = PDO_FETCH_BOTH;
  }

  m_dbh->auto_commit = pdo_attr_lval(options, PDO_ATTR_AUTOCOMMIT, 1);

  if (!call_factory) {
    /* we got a persistent guy from our cache */
    for (ArrayIter iter(options); iter; ++iter) {
      t_setattribute(iter.first().toInt64(), iter.second());
    }
  } else if (m_dbh.get()) {
    if (is_persistent) {
      ASSERT(!shashkey.empty());
      g_persistentObjects->set(PDOConnection::PersistentKey, shashkey.data(),
                               m_dbh.get());
      s_pdo_request_data->m_persistent_connections.insert(m_dbh.get());
    }

    m_dbh->driver = driver;
    for (ArrayIter iter(options); iter; ++iter) {
      t_setattribute(iter.first().toInt64(), iter.second());
    }
  }
}


Variant c_pdo::t_prepare(CStrRef statement,
                         CArrRef options /* = null_array */) {
  ASSERT(m_dbh->driver);
  strcpy(m_dbh->error_code, PDO_ERR_NONE);
  m_dbh->query_stmt = NULL;

  String clsname;
  Variant ctor_args;
  if (options.exists(PDO_ATTR_STATEMENT_CLASS)) {
    Variant opt = options[PDO_ATTR_STATEMENT_CLASS];
    if (!valid_statement_class(m_dbh, opt, clsname, ctor_args)) {
      return false;
    }
  } else {
    clsname = m_dbh->def_stmt_clsname;
    ctor_args = m_dbh->def_stmt_ctor_args;
  }

  Object ret = pdo_stmt_instantiate(m_dbh, clsname, ctor_args);
  if (ret.isNull()) {
    pdo_raise_impl_error
      (m_dbh, NULL, "HY000",
       "failed to instantiate user-supplied statement class");
    PDO_HANDLE_DBH_ERR(m_dbh);
    return false;
  }
  c_pdostatement *pdostmt = ret.getTyped<c_pdostatement>();

  if (m_dbh->preparer(statement, &pdostmt->m_stmt, options)) {
    PDOStatement *stmt = pdostmt->m_stmt.get();
    ASSERT(stmt);

    /* unconditionally keep this for later reference */
    stmt->query_string = statement;
    stmt->default_fetch_type = m_dbh->default_fetch_type;
    stmt->dbh = m_dbh;

    pdo_stmt_construct(stmt, ret, clsname, ctor_args);
    return ret;
  }

  PDO_HANDLE_DBH_ERR(m_dbh);
  return false;
}

bool c_pdo::t_begintransaction() {
  if (m_dbh->in_txn) {
    throw_pdo_exception(null, null, "There is already an active transaction");
  }
  if (m_dbh->begin()) {
    m_dbh->in_txn = 1;
    return true;
  }
  if (strcmp(m_dbh->error_code, PDO_ERR_NONE)) {
    pdo_handle_error(m_dbh, NULL);
  }
  return false;
}

bool c_pdo::t_commit() {
  ASSERT(m_dbh->driver);
  if (!m_dbh->in_txn) {
    throw_pdo_exception(null, null, "There is no active transaction");
  }
  if (m_dbh->commit()) {
    m_dbh->in_txn = 0;
    return true;
  }
  PDO_HANDLE_DBH_ERR(m_dbh);
  return false;
}

bool c_pdo::t_rollback() {
  ASSERT(m_dbh->driver);
  if (!m_dbh->in_txn) {
    throw_pdo_exception(null, null, "There is no active transaction");
  }
  if (m_dbh->rollback()) {
    m_dbh->in_txn = 0;
    return true;
  }
  PDO_HANDLE_DBH_ERR(m_dbh);
  return false;
}

bool c_pdo::t_setattribute(int64 attribute, CVarRef value) {
  ASSERT(m_dbh->driver);

#define PDO_LONG_PARAM_CHECK                                           \
  if (!value.isInteger() && !value.isString() && !value.isBoolean()) { \
    pdo_raise_impl_error(m_dbh, NULL, "HY000",                         \
                         "attribute value must be an integer");        \
    PDO_HANDLE_DBH_ERR(m_dbh);                                         \
    return false;                                                      \
  }                                                                    \

  switch (attribute) {
  case PDO_ATTR_ERRMODE:
    PDO_LONG_PARAM_CHECK;
    switch (value.toInt64()) {
    case PDO_ERRMODE_SILENT:
    case PDO_ERRMODE_WARNING:
    case PDO_ERRMODE_EXCEPTION:
      m_dbh->error_mode = (PDOErrorMode)value.toInt64();
      return true;
    default:
      pdo_raise_impl_error(m_dbh, NULL, "HY000", "invalid error mode");
      PDO_HANDLE_DBH_ERR(m_dbh);
      return false;
    }
    return false;

  case PDO_ATTR_CASE:
    PDO_LONG_PARAM_CHECK;
    switch (value.toInt64()) {
    case PDO_CASE_NATURAL:
    case PDO_CASE_UPPER:
    case PDO_CASE_LOWER:
      m_dbh->desired_case = (PDOCaseConversion)value.toInt64();
      return true;
    default:
      pdo_raise_impl_error(m_dbh, NULL, "HY000",
                           "invalid case folding mode");
      PDO_HANDLE_DBH_ERR(m_dbh);
      return false;
    }
    return false;

  case PDO_ATTR_ORACLE_NULLS:
    PDO_LONG_PARAM_CHECK;
    m_dbh->oracle_nulls = value.toInt64();
    return true;

  case PDO_ATTR_DEFAULT_FETCH_MODE:
    if (value.isArray()) {
      if (value.toArray().exists(0)) {
        Variant tmp = value[0];
        if (tmp.isInteger() && ((tmp.toInt64() == PDO_FETCH_INTO ||
                                 tmp.toInt64() == PDO_FETCH_CLASS))) {
          pdo_raise_impl_error(m_dbh, NULL, "HY000",
                               "FETCH_INTO and FETCH_CLASS are not yet "
                               "supported as default fetch modes");
          return false;
        }
      }
    } else {
      PDO_LONG_PARAM_CHECK;
    }
    if (value.toInt64() == PDO_FETCH_USE_DEFAULT) {
      pdo_raise_impl_error(m_dbh, NULL, "HY000", "invalid fetch mode type");
      return false;
    }
    m_dbh->default_fetch_type = (PDOFetchType)value.toInt64();
    return true;

  case PDO_ATTR_STRINGIFY_FETCHES:
    PDO_LONG_PARAM_CHECK;
    m_dbh->stringify = value.toInt64() ? 1 : 0;
    return true;

  case PDO_ATTR_STATEMENT_CLASS:
    {
      if (m_dbh->is_persistent) {
        pdo_raise_impl_error(m_dbh, NULL, "HY000",
                             "PDO::ATTR_STATEMENT_CLASS cannot be used "
                             "with persistent PDO instances");
        PDO_HANDLE_DBH_ERR(m_dbh);
        return false;
      }
      String clsname;
      if (!valid_statement_class(m_dbh, value, clsname,
                                 m_dbh->def_stmt_ctor_args)) {
        return false;
      }
      m_dbh->def_stmt_clsname = clsname.c_str();
      return true;
    }
  }

  if (m_dbh->support(PDOConnection::MethodSetAttribute)) {
    strcpy(m_dbh->error_code, PDO_ERR_NONE);
    m_dbh->query_stmt = NULL;
    if (m_dbh->setAttribute(attribute, value)) {
      return true;
    }
  }

  if (attribute == PDO_ATTR_AUTOCOMMIT) {
    throw_pdo_exception(null, null,
                        "The auto-commit mode cannot be changed for this "
                        "driver");
  } else if (!m_dbh->support(PDOConnection::MethodSetAttribute)) {
    pdo_raise_impl_error(m_dbh, NULL, "IM001",
                         "driver does not support setting attributes");
  } else {
    PDO_HANDLE_DBH_ERR(m_dbh);
  }
  return false;
}

Variant c_pdo::t_getattribute(int64 attribute) {
  ASSERT(m_dbh->driver);
  strcpy(m_dbh->error_code, PDO_ERR_NONE);
  m_dbh->query_stmt = NULL;

  /* handle generic PDO-level atributes */
  switch (attribute) {
  case PDO_ATTR_PERSISTENT:
    return (bool)m_dbh->is_persistent;

  case PDO_ATTR_CASE:
    return (int64)m_dbh->desired_case;

  case PDO_ATTR_ORACLE_NULLS:
    return (int64)m_dbh->oracle_nulls;

  case PDO_ATTR_ERRMODE:
    return (int64)m_dbh->error_mode;

  case PDO_ATTR_DRIVER_NAME:
    return String(m_dbh->driver->getName());

  case PDO_ATTR_STATEMENT_CLASS: {
    Array ret;
    ret.append(String(m_dbh->def_stmt_clsname));
    if (!m_dbh->def_stmt_ctor_args.isNull()) {
      ret.append(m_dbh->def_stmt_ctor_args);
    }
    return ret;
  }
  case PDO_ATTR_DEFAULT_FETCH_MODE:
    return (int64)m_dbh->default_fetch_type;
  }

  if (!m_dbh->support(PDOConnection::MethodGetAttribute)) {
    pdo_raise_impl_error(m_dbh, NULL, "IM001",
                         "driver does not support getting attributes");
    return false;
  }

  Variant ret;
  switch (m_dbh->getAttribute(attribute, ret)) {
  case -1:
    PDO_HANDLE_DBH_ERR(m_dbh);
    return false;
  case 0:
    pdo_raise_impl_error(m_dbh, NULL, "IM001",
                         "driver does not support that attribute");
    return false;
  }
  return ret;
}

Variant c_pdo::t_exec(CStrRef query) {
  if (query.empty()) {
    pdo_raise_impl_error(m_dbh, NULL, "HY000",
                         "trying to execute an empty query");
    return false;
  }

  ASSERT(m_dbh->driver);
  strcpy(m_dbh->error_code, PDO_ERR_NONE);
  m_dbh->query_stmt = NULL;

  int64 ret = m_dbh->doer(query);
  if (ret == -1) {
    PDO_HANDLE_DBH_ERR(m_dbh);
    return false;
  }
  return ret;
}

Variant c_pdo::t_lastinsertid(CStrRef seqname /* = null_string */) {
  ASSERT(m_dbh->driver);
  strcpy(m_dbh->error_code, PDO_ERR_NONE);
  m_dbh->query_stmt = NULL;

  if (!m_dbh->support(PDOConnection::MethodLastId)) {
    pdo_raise_impl_error(m_dbh, NULL, "IM001",
                         "driver does not support lastInsertId()");
    return false;
  }

  String ret = m_dbh->lastId(seqname.data());
  if (ret.empty()) {
    PDO_HANDLE_DBH_ERR(m_dbh);
    return false;
  }
  return ret;
}

Variant c_pdo::t_errorcode() {
  ASSERT(m_dbh->driver);
  if (m_dbh->query_stmt) {
    return String(m_dbh->query_stmt->error_code, CopyString);
  }

  if (m_dbh->error_code[0] == '\0') {
    return null;
  }

  /**
   * Making sure that we fallback to the default implementation
   * if the dbh->error_code is not null.
   */
  return String(m_dbh->error_code, CopyString);
}

Array c_pdo::t_errorinfo() {
  ASSERT(m_dbh->driver);

  Array ret;
  if (m_dbh->query_stmt) {
    ret.append(String(m_dbh->query_stmt->error_code, CopyString));
  } else {
    ret.append(String(m_dbh->error_code, CopyString));
  }

  if (m_dbh->support(PDOConnection::MethodFetchErr)) {
    m_dbh->fetchErr(m_dbh->query_stmt, ret);
  }

  /**
   * In order to be consistent, we have to make sure we add the good amount
   * of nulls depending on the current number of elements. We make a simple
   * difference and add the needed elements
   */
  int error_count = ret.size();
  int error_expected_count = 3;
  if (error_expected_count > error_count) {
    int error_count_diff = error_expected_count - error_count;
    for (int i = 0; i < error_count_diff; i++) {
      ret.append(null);
    }
  }
  return ret;
}

Variant c_pdo::t_query(CStrRef sql) {
  ASSERT(m_dbh->driver);
  strcpy(m_dbh->error_code, PDO_ERR_NONE);
  m_dbh->query_stmt = NULL;

  Object ret = pdo_stmt_instantiate(m_dbh, m_dbh->def_stmt_clsname,
                                    m_dbh->def_stmt_ctor_args);
  if (ret.isNull()) {
    pdo_raise_impl_error
      (m_dbh, NULL, "HY000",
       "failed to instantiate user supplied statement class");
    return null;
  }
  c_pdostatement *pdostmt = ret.getTyped<c_pdostatement>();

  if (m_dbh->preparer(sql, &pdostmt->m_stmt, Array())) {
    PDOStatement *stmt = pdostmt->m_stmt.get();
    ASSERT(stmt);

    /* unconditionally keep this for later reference */
    stmt->query_string = sql;
    stmt->default_fetch_type = m_dbh->default_fetch_type;
    stmt->active_query_string = stmt->query_string;
    stmt->dbh = m_dbh;
    stmt->lazy_object_ref = null;

    strcpy(stmt->error_code, PDO_ERR_NONE);
    if (pdo_stmt_set_fetch_mode(stmt, 0, 1, Array())) {
      /* now execute the statement */
      strcpy(stmt->error_code, PDO_ERR_NONE);
      if (stmt->executer()) {
        int ok = 1;
        if (!stmt->executed) {
          if (stmt->dbh->alloc_own_columns) {
            ok = pdo_stmt_describe_columns(stmt);
          }
          stmt->executed = 1;
        }
        if (ok) {
          pdo_stmt_construct(stmt, ret, m_dbh->def_stmt_clsname,
                             m_dbh->def_stmt_ctor_args);
          return ret;
        }
      }
    }
    /* something broke */
    m_dbh->query_stmt = stmt;
    PDO_HANDLE_STMT_ERR(stmt);
  } else {
    PDO_HANDLE_DBH_ERR(m_dbh);
  }

  return false;
}

Variant c_pdo::t_quote(CStrRef str, int64 paramtype /* = q_pdo_PARAM_STR */) {
  ASSERT(m_dbh->driver);
  strcpy(m_dbh->error_code, PDO_ERR_NONE);
  m_dbh->query_stmt = NULL;

  if (!m_dbh->support(PDOConnection::MethodQuoter)) {
    pdo_raise_impl_error(m_dbh, NULL, "IM001",
                         "driver does not support quoting");
    return false;
  }

  String quoted;
  if (m_dbh->quoter(str, quoted, (PDOParamType)paramtype)) {
    return quoted;
  }
  PDO_HANDLE_DBH_ERR(m_dbh);
  return false;
}

Variant c_pdo::t___wakeup() {
  throw_pdo_exception(null, null,
                      "You cannot serialize or unserialize PDO instances");
  return null;
}

Variant c_pdo::t___sleep() {
  throw_pdo_exception(null, null,
                      "You cannot serialize or unserialize PDO instances");
  return null;
}

Array c_pdo::t_getavailabledrivers() {
  return f_pdo_drivers();
}

Variant c_pdo::t___destruct() {
  return null;
}

///////////////////////////////////////////////////////////////////////////////

static inline bool rewrite_name_to_position(sp_PDOStatement stmt,
                                            PDOBoundParam *param) {
  if (!stmt->bound_param_map.empty()) {
    /* rewriting :name to ? style.
     * We need to fixup the parameter numbers on the parameters.
     * If we find that a given named parameter has been used twice,
     * we will raise an error, as we can't be sure that it is safe
     * to bind multiple parameters onto the same zval in the underlying
     * driver */
    if (stmt->named_rewrite_template) {
      /* this is not an error here */
      return true;
    }
    if (param->name.empty()) {
      /* do the reverse; map the parameter number to the name */
      if (stmt->bound_param_map.exists(param->paramno)) {
        param->name = stmt->bound_param_map[param->paramno].toString();
        return true;
      }
      pdo_raise_impl_error(stmt->dbh, stmt, "HY093",
                           "parameter was not defined");
      return false;
    }

    int position = 0;
    for (ArrayIter iter(stmt->bound_param_map); iter; ++iter, ++position) {
      if (iter.second().toString() == param->name) {
        if (param->paramno >= 0) {
          pdo_raise_impl_error
            (stmt->dbh, stmt, "IM001",
             "PDO refuses to handle repeating the same :named parameter "
             "for multiple positions with this driver, as it might be "
             "unsafe to do so.  Consider using a separate name for each "
             "parameter instead");
          return true;
        }
        param->paramno = position;
        return true;
      }
    }
    pdo_raise_impl_error(stmt->dbh, stmt, "HY093",
                         "parameter was not defined");
    return false;
  }
  return true;
}

/* trigger callback hook for parameters */
static bool dispatch_param_event(sp_PDOStatement stmt,
                                 PDOParamEvent event_type) {
  if (!stmt->support(PDOStatement::MethodParamHook)) {
    return true;
  }
  for (ArrayIter iter(stmt->bound_params); iter; ++iter) {
    PDOBoundParam *param = iter.second().toObject().getTyped<PDOBoundParam>();
    if (!stmt->paramHook(param, event_type)) {
      return false;
    }
  }
  for (ArrayIter iter(stmt->bound_columns); iter; ++iter) {
    PDOBoundParam *param = iter.second().toObject().getTyped<PDOBoundParam>();
    if (!stmt->paramHook(param, event_type)) {
      return false;
    }
  }
  return true;
}

static void get_lazy_object(sp_PDOStatement stmt, Variant &ret) {
  if (stmt->lazy_object_ref.isNull()) {
    stmt->lazy_object_ref = stmt;
  }
  ret = stmt->lazy_object_ref;
}

static bool really_register_bound_param(PDOBoundParam *param,
                                        sp_PDOStatement stmt, bool is_param) {
  Array &hash = is_param ? stmt->bound_params : stmt->bound_columns;

  if (PDO_PARAM_TYPE(param->param_type) == PDO_PARAM_STR &&
      param->max_value_len <= 0 && !param->parameter.isNull()) {
    param->parameter = param->parameter.toString();
  } else if (PDO_PARAM_TYPE(param->param_type) == PDO_PARAM_INT &&
             param->parameter.isBoolean()) {
    param->parameter = param->parameter.toInt64();
  } else if (PDO_PARAM_TYPE(param->param_type) == PDO_PARAM_BOOL &&
             param->parameter.isInteger()) {
    param->parameter = param->parameter.toBoolean();
  }
  param->stmt = stmt;
  param->is_param = is_param;

  if (!is_param && !param->name.empty() && !stmt->columns.empty()) {
    /* try to map the name to the column */
    for (int i = 0; i < stmt->column_count; i++) {
      if (stmt->columns[i].toObject().getTyped<PDOColumn>()->name ==
          param->name) {
        param->paramno = i;
        break;
      }
    }

    /* if you prepare and then execute passing an array of params keyed by
       names, then this will trigger, and we don't want that */
    if (param->paramno == -1) {
      raise_warning("Did not found column name '%s' in the defined columns;"
                    " it will not be bound", param->name.data());
    }
  }

  if (is_param && !param->name.empty() && param->name[0] != ':') {
    param->name = String(":") + param->name;
  }

  if (is_param && !rewrite_name_to_position(stmt, param)) {
    param->name.reset();
    return false;
  }

  /* ask the driver to perform any normalization it needs on the
   * parameter name.  Note that it is illegal for the driver to take
   * a reference to param, as it resides in transient storage only
   * at this time. */
  if (stmt->support(PDOStatement::MethodParamHook)) {
    if (!stmt->paramHook(param, PDO_PARAM_EVT_NORMALIZE)) {
      param->name.reset();
      return false;
    }
  }

  /* delete any other parameter registered with this number.
   * If the parameter is named, it will be removed and correctly
   * disposed of by the hash_update call that follows */
  if (param->paramno >= 0) {
    hash.remove(param->paramno);
  }

  /* allocate storage for the parameter, keyed by its "canonical" name */
  if (!param->name.empty()) {
    hash.set(param->name, param);
  } else {
    hash.set(param->paramno, param);
  }

  /* tell the driver we just created a parameter */
  if (stmt->support(PDOStatement::MethodParamHook)) {
    if (!stmt->paramHook(param, PDO_PARAM_EVT_ALLOC)) {
      /* undo storage allocation; the hash will free the parameter
       * name if required */
      if (!param->name.empty()) {
        hash.remove(param->name);
      } else {
        hash.remove(param->paramno);
      }
      /* param->parameter is freed by hash dtor */
      param->parameter.reset();
      return false;
    }
  }
  return true;
}

static inline void fetch_value(sp_PDOStatement stmt, Variant &dest, int colno,
                               int *type_override) {
  PDOColumn *col = stmt->columns[colno].toObject().getTyped<PDOColumn>();
  int type = PDO_PARAM_TYPE(col->param_type);
  int new_type =  type_override ? PDO_PARAM_TYPE(*type_override) : type;

  stmt->getColumn(colno, dest);

  if (type != new_type) {
    switch (new_type) {
    case PDO_PARAM_INT:  dest = dest.toInt64();   break;
    case PDO_PARAM_BOOL: dest = dest.toBoolean(); break;
    case PDO_PARAM_STR:  dest = dest.toString();  break;
    case PDO_PARAM_NULL: dest = null;             break;
    }
  }
  if (stmt->dbh->stringify && (dest.isInteger() || dest.isDouble())) {
    dest = dest.toString();
  }
  if (dest.isNull() && stmt->dbh->oracle_nulls == PDO_NULL_TO_STRING) {
    dest = "";
  }
}

static bool do_fetch_common(sp_PDOStatement stmt, PDOFetchOrientation ori,
                            long offset, bool do_bind) {
  if (!stmt->executed) {
    return false;
  }
  if (!dispatch_param_event(stmt, PDO_PARAM_EVT_FETCH_PRE)) {
    return false;
  }
  if (!stmt->fetcher(ori, offset)) {
    return false;
  }
  /* some drivers might need to describe the columns now */
  if (stmt->columns.empty() && !pdo_stmt_describe_columns(stmt)) {
    return false;
  }
  if (!dispatch_param_event(stmt, PDO_PARAM_EVT_FETCH_POST)) {
    return false;
  }

  if (do_bind && !stmt->bound_columns.empty()) {
    /* update those bound column variables now */
    for (ArrayIter iter(stmt->bound_columns); iter; ++iter) {
      PDOBoundParam *param =
        iter.second().toObject().getTyped<PDOBoundParam>();
      if (param->paramno >= 0) {
        param->parameter.reset();
        /* set new value */
        fetch_value(stmt, param->parameter, param->paramno,
                    (int *)&param->param_type);
        /* TODO: some smart thing that avoids duplicating the value in the
         * general loop below.  For now, if you're binding output columns,
         * it's better to use LAZY or BOUND fetches if you want to shave
         * off those cycles */
      }
    }
  }

  return true;
}

static bool do_fetch_func_prepare(sp_PDOStatement stmt) {
  if (!f_is_callable(stmt->fetch.func)) {
    pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                         "user-supplied function must be a valid callback");
    return false;
  }
  return true;
}

/* perform a fetch.  If do_bind is true, update any bound columns.
 * If return_value is not null, store values into it according to HOW. */
static bool do_fetch(sp_PDOStatement stmt, bool do_bind, Variant &ret,
                     PDOFetchType how, PDOFetchOrientation ori,
                     long offset, Variant *return_all) {
  if (how == PDO_FETCH_USE_DEFAULT) {
    how = stmt->default_fetch_type;
  }
  int flags = how & PDO_FETCH_FLAGS;
  how = (PDOFetchType)(how & ~PDO_FETCH_FLAGS);

  if (!do_fetch_common(stmt, ori, offset, do_bind)) {
    return false;
  }

  if (how == PDO_FETCH_BOUND) {
    ret = true;
    return true;
  }

  int colno;
  if ((flags & PDO_FETCH_GROUP) && stmt->fetch.column == -1) {
    colno = 1;
  } else {
    colno = stmt->fetch.column;
  }

  if (how == PDO_FETCH_LAZY) {
    get_lazy_object(stmt, ret);
    return true;
  }

  String clsname, old_clsname;
  Variant old_ctor_args;
  ret = false;
  int i = 0;
  switch (how) {
  case PDO_FETCH_USE_DEFAULT:
  case PDO_FETCH_ASSOC:
  case PDO_FETCH_BOTH:
  case PDO_FETCH_NUM:
  case PDO_FETCH_NAMED:
    ret = Array::Create();
    break;

  case PDO_FETCH_KEY_PAIR:
    if (stmt->column_count != 2) {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                           "PDO::FETCH_KEY_PAIR fetch mode requires the "
                           "result set to contain extactly 2 columns.");
      return false;
    }
    if (!return_all) {
      ret = Array::Create();
    }
    break;

  case PDO_FETCH_COLUMN:
    if (colno >= 0 && colno < stmt->column_count) {
      if (flags == PDO_FETCH_GROUP && stmt->fetch.column == -1) {
        fetch_value(stmt, ret, 1, NULL);
      } else if (flags == PDO_FETCH_GROUP && colno) {
        fetch_value(stmt, ret, 0, NULL);
      } else {
        fetch_value(stmt, ret, colno, NULL);
      }
      if (!return_all) {
        return true;
      }
      break;
    } else {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "Invalid column index");
    }
    return false;

  case PDO_FETCH_OBJ:
    ret = NEW(c_stdclass)();
    break;

  case PDO_FETCH_CLASS:
    if (flags & PDO_FETCH_CLASSTYPE) {
      old_clsname = stmt->fetch.clsname;
      old_ctor_args = stmt->fetch.ctor_args;

      Variant val;
      fetch_value(stmt, val, i++, NULL);
      if (!val.isNull()) {
        if (!f_class_exists(val)) {
          stmt->fetch.clsname = "stdclass";
        } else {
          stmt->fetch.clsname = val.toString();
        }
      }

      do_fetch_class_prepare(stmt);
    }
    clsname = stmt->fetch.clsname;
    if (clsname.empty()) {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                           "No fetch class specified");
      return false;
    }
    if ((flags & PDO_FETCH_SERIALIZE) == 0) {
      ret = create_object(clsname, Array(), false);
      if (!do_fetch_class_prepare(stmt)) {
        return false;
      }
      if (stmt->fetch.constructor && (flags & PDO_FETCH_PROPS_LATE)) {
        ret.toObject()->o_invoke(stmt->fetch.constructor,
                                 stmt->fetch.ctor_args, -1);
      }
    }
    break;

  case PDO_FETCH_INTO:
    if (stmt->fetch.into.isNull()) {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                           "No fetch-into object specified.");
      return false;
    }

    ret = stmt->fetch.into;
    if (ret.instanceof("stdclass")) {
      how = PDO_FETCH_OBJ;
    }
    break;

  case PDO_FETCH_FUNC:
    if (stmt->fetch.func.empty()) {
      pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                           "No fetch function specified");
      return false;
    }
    if (!do_fetch_func_prepare(stmt)) {
      return false;
    }
    break;

  default:
    ASSERT(false);
    return false;
  }

  Variant grp_val;
  if (return_all && how != PDO_FETCH_KEY_PAIR) {
    if (flags == PDO_FETCH_GROUP && how == PDO_FETCH_COLUMN &&
        stmt->fetch.column > 0) {
      fetch_value(stmt, grp_val, colno, NULL);
    } else {
      fetch_value(stmt, grp_val, i, NULL);
    }
    grp_val = grp_val.toString();
    if (how == PDO_FETCH_COLUMN) {
      i = stmt->column_count; /* no more data to fetch */
    } else {
      i++;
    }
  }

  for (int idx = 0; i < stmt->column_count; i++, idx++) {
    String name = stmt->columns[i].toObject().getTyped<PDOColumn>()->name;
    Variant val;
    fetch_value(stmt, val, i, NULL);

    switch (how) {
    case PDO_FETCH_ASSOC:
      ret.set(name, val);
      break;

    case PDO_FETCH_KEY_PAIR: {
      Variant tmp;
      fetch_value(stmt, tmp, ++i, NULL);
      if (return_all) {
        return_all->set(val, tmp);
      } else {
        ret.set(val, tmp);
      }
      return true;
    }
    case PDO_FETCH_USE_DEFAULT:
    case PDO_FETCH_BOTH:
      ret.set(name, val);
      ret.append(val);
      break;

    case PDO_FETCH_NAMED: {
      /* already have an item with this name? */
      if (ret.toArray().exists(name)) {
        Variant &curr_val = ret.lvalAt(name);
        if (!curr_val.isArray()) {
          Array arr = Array::Create();
          arr.append(curr_val);
          arr.append(val);
          ret.set(name, arr);
        } else {
          curr_val.append(val);
        }
      } else {
        ret.set(name, val);
      }
      break;
    }
    case PDO_FETCH_NUM:
      ret.append(val);
      break;

    case PDO_FETCH_OBJ:
    case PDO_FETCH_INTO:
      ret.toObject()->set(name, val);
      break;

    case PDO_FETCH_CLASS:
      if ((flags & PDO_FETCH_SERIALIZE) == 0 || idx) {
        ret.toObject()->set(name, val);
      } else {
#ifdef MBO_0
        ret = f_unserialize(val);
        if (same(ret, false)) {
          pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                               "cannot unserialize data");
          return false;
        }
#endif
        // hzhao: not sure how we support class serialization
        pdo_raise_impl_error(stmt->dbh, stmt, "HY000",
                             "cannot unserialize class");
        return false;
      }
      break;

    case PDO_FETCH_FUNC:
      stmt->fetch.values.set(idx, val);
      break;

    default:
      pdo_raise_impl_error(stmt->dbh, stmt, "22003", "mode is out of range");
      return false;
    }
  }

  switch (how) {
  case PDO_FETCH_CLASS:
    if (stmt->fetch.constructor &&
        !(flags & (PDO_FETCH_PROPS_LATE | PDO_FETCH_SERIALIZE))) {
      ret.toObject()->o_invoke(stmt->fetch.constructor, stmt->fetch.ctor_args,
                              -1);
    }
    if (flags & PDO_FETCH_CLASSTYPE) {
      stmt->fetch.clsname = old_clsname;
      stmt->fetch.ctor_args = old_ctor_args;
    }
    break;

  case PDO_FETCH_FUNC:
    ret = f_call_user_func_array(stmt->fetch.func, stmt->fetch.values);
    break;

  default:
    break;
  }

  if (return_all) {
    if ((flags & PDO_FETCH_UNIQUE) == PDO_FETCH_UNIQUE) {
      return_all->set(grp_val, ret);
    } else {
      return_all->lvalAt(grp_val).append(ret);
    }
  }

  return true;
}

static int register_bound_param(CVarRef paramno, CVarRef param, int64 type,
                                int64 max_value_len, CVarRef driver_params,
                                sp_PDOStatement stmt, bool is_param) {
  SmartObject<PDOBoundParam> p(new PDOBoundParam);

  if (paramno.isNumeric()) {
    p->paramno = paramno.toInt64();
  } else {
    p->paramno = -1;
    p->name = paramno.toString();
  }

  p->parameter = param;
  p->param_type = (PDOParamType)type;
  p->max_value_len = max_value_len;
  p->driver_params = driver_params;

  if (p->paramno > 0) {
    --p->paramno; /* make it zero-based internally */
  } else if (p->name.empty()) {
    pdo_raise_impl_error(stmt->dbh, stmt, "HY093",
                         "Columns/Parameters are 1-based");
    return false;
  }

  if (!really_register_bound_param(p.get(), stmt, is_param)) {
    p->parameter.reset();
    return false;
  }
  return true;
}

static bool generic_stmt_attr_get(sp_PDOStatement stmt, Variant &ret,
                                  long attr) {
  if (attr == PDO_ATTR_EMULATE_PREPARES) {
    ret = (bool)(stmt->supports_placeholders == PDO_PLACEHOLDER_NONE);
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// SQL parser

#define PDO_PARSER_TEXT 1
#define PDO_PARSER_BIND 2
#define PDO_PARSER_BIND_POS 3
#define PDO_PARSER_EOI 4

#define RET(i) {s->cur = cursor; return i; }
#define SKIP_ONE(i) {s->cur = s->tok + 1; return 1; }

#define YYCTYPE         unsigned char
#define YYCURSOR        cursor
#define YYLIMIT         cursor
#define YYMARKER        s->ptr
#define YYFILL(n)

typedef struct Scanner {
  char *ptr, *cur, *tok;
} Scanner;

static int scan(Scanner *s) {
  char *cursor = s->cur;
  s->tok = cursor;

{
  YYCTYPE yych;

  if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
  yych = *YYCURSOR;
  switch (yych) {
  case 0x00:  goto yy11;
  case '"':  goto yy2;
  case '\'':  goto yy4;
  case ':':  goto yy5;
  case '?':  goto yy6;
  default:  goto yy8;
  }
yy2:
  yych = *(YYMARKER = ++YYCURSOR);
  if (yych >= 0x01) goto yy26;
yy3:
  { SKIP_ONE(PDO_PARSER_TEXT); }
yy4:
  yych = *(YYMARKER = ++YYCURSOR);
  if (yych <= 0x00) goto yy3;
  goto yy20;
yy5:
  yych = *++YYCURSOR;
  switch (yych) {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
  case 'G':
  case 'H':
  case 'I':
  case 'J':
  case 'K':
  case 'L':
  case 'M':
  case 'N':
  case 'O':
  case 'P':
  case 'Q':
  case 'R':
  case 'S':
  case 'T':
  case 'U':
  case 'V':
  case 'W':
  case 'X':
  case 'Y':
  case 'Z':
  case '_':
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
  case 'g':
  case 'h':
  case 'i':
  case 'j':
  case 'k':
  case 'l':
  case 'm':
  case 'n':
  case 'o':
  case 'p':
  case 'q':
  case 'r':
  case 's':
  case 't':
  case 'u':
  case 'v':
  case 'w':
  case 'x':
  case 'y':
  case 'z':  goto yy16;
  case ':':
  case '?':  goto yy13;
  default:  goto yy3;
  }
yy6:
  ++YYCURSOR;
  switch ((yych = *YYCURSOR)) {
  case ':':
  case '?':  goto yy13;
  default:  goto yy7;
  }
yy7:
  { RET(PDO_PARSER_BIND_POS); }
yy8:
  ++YYCURSOR;
  if (YYLIMIT <= YYCURSOR) YYFILL(1);
  yych = *YYCURSOR;
  switch (yych) {
  case 0x00:
  case '"':
  case '\'':
  case ':':
  case '?':  goto yy10;
  default:  goto yy8;
  }
yy10:
  { RET(PDO_PARSER_TEXT); }
yy11:
  ++YYCURSOR;
  { RET(PDO_PARSER_EOI); }
yy13:
  ++YYCURSOR;
  if (YYLIMIT <= YYCURSOR) YYFILL(1);
  yych = *YYCURSOR;
  switch (yych) {
  case ':':
  case '?':  goto yy13;
  default:  goto yy15;
  }
yy15:
  { RET(PDO_PARSER_TEXT); }
yy16:
  ++YYCURSOR;
  if (YYLIMIT <= YYCURSOR) YYFILL(1);
  yych = *YYCURSOR;
  switch (yych) {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
  case 'G':
  case 'H':
  case 'I':
  case 'J':
  case 'K':
  case 'L':
  case 'M':
  case 'N':
  case 'O':
  case 'P':
  case 'Q':
  case 'R':
  case 'S':
  case 'T':
  case 'U':
  case 'V':
  case 'W':
  case 'X':
  case 'Y':
  case 'Z':
  case '_':
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
  case 'g':
  case 'h':
  case 'i':
  case 'j':
  case 'k':
  case 'l':
  case 'm':
  case 'n':
  case 'o':
  case 'p':
  case 'q':
  case 'r':
  case 's':
  case 't':
  case 'u':
  case 'v':
  case 'w':
  case 'x':
  case 'y':
  case 'z':  goto yy16;
  default:  goto yy18;
  }
yy18:
  { RET(PDO_PARSER_BIND); }
yy19:
  ++YYCURSOR;
  if (YYLIMIT <= YYCURSOR) YYFILL(1);
  yych = *YYCURSOR;
yy20:
  switch (yych) {
  case 0x00:  goto yy21;
  case '\'':  goto yy23;
  case '\\':  goto yy22;
  default:  goto yy19;
  }
yy21:
  YYCURSOR = YYMARKER;
  goto yy3;
yy22:
  ++YYCURSOR;
  if (YYLIMIT <= YYCURSOR) YYFILL(1);
  yych = *YYCURSOR;
  if (yych <= 0x00) goto yy21;
  goto yy19;
yy23:
  ++YYCURSOR;
  { RET(PDO_PARSER_TEXT); }
yy25:
  ++YYCURSOR;
  if (YYLIMIT <= YYCURSOR) YYFILL(1);
  yych = *YYCURSOR;
yy26:
  switch (yych) {
  case 0x00:  goto yy21;
  case '"':  goto yy28;
  case '\\':  goto yy27;
  default:  goto yy25;
  }
yy27:
  ++YYCURSOR;
  if (YYLIMIT <= YYCURSOR) YYFILL(1);
  yych = *YYCURSOR;
  if (yych <= 0x00) goto yy21;
  goto yy25;
yy28:
  ++YYCURSOR;
  { RET(PDO_PARSER_TEXT); }
}

}

struct placeholder {
  char *pos;
  int len;
  int bindno;
  String quoted;  /* quoted value */
  struct placeholder *next;
};

int pdo_parse_params(PDOStatement *stmt, CStrRef in, String &out) {
  Scanner s;
  const char *ptr;
  char *newbuffer;
  int t;
  int bindno = 0;
  int ret = 0;
  int newbuffer_len;
  Array params;
  PDOBoundParam *param;
  int query_type = PDO_PLACEHOLDER_NONE;
  struct placeholder *placeholders = NULL, *placetail = NULL, *plc = NULL;

  s.cur = (char*)in.data();

  /* phase 1: look for args */
  while ((t = scan(&s)) != PDO_PARSER_EOI) {
    if (t == PDO_PARSER_BIND || t == PDO_PARSER_BIND_POS) {
      if (t == PDO_PARSER_BIND) {
        int len = s.cur - s.tok;
        if ((in.data() < (s.cur - len)) && isalnum(*(s.cur - len - 1))) {
          continue;
        }
        query_type |= PDO_PLACEHOLDER_NAMED;
      } else {
        query_type |= PDO_PLACEHOLDER_POSITIONAL;
      }

      plc = (placeholder*)malloc(sizeof(*plc));
      memset(plc, 0, sizeof(*plc));
      plc->next = NULL;
      plc->pos = s.tok;
      plc->len = s.cur - s.tok;
      plc->bindno = bindno++;

      if (placetail) {
        placetail->next = plc;
      } else {
        placeholders = plc;
      }
      placetail = plc;
    }
  }

  if (bindno == 0) {
    /* nothing to do; good! */
    return 0;
  }

  /* did the query make sense to me? */
  if (query_type == (PDO_PLACEHOLDER_NAMED|PDO_PLACEHOLDER_POSITIONAL)) {
    /* they mixed both types; punt */
    pdo_raise_impl_error(stmt->dbh, stmt, "HY093",
                         "mixed named and positional parameters");
    ret = -1;
    goto clean_up;
  }

  if ((int)stmt->supports_placeholders == query_type &&
      !stmt->named_rewrite_template) {
    /* query matches native syntax */
    ret = 0;
    goto clean_up;
  }

  if (stmt->named_rewrite_template) {
    /* magic/hack.
     * We we pretend that the query was positional even if
     * it was named so that we fall into the
     * named rewrite case below.  Not too pretty,
     * but it works. */
    query_type = PDO_PLACEHOLDER_POSITIONAL;
  }

  params = stmt->bound_params;

  /* Do we have placeholders but no bound params */
  if (bindno && params.empty() &&
      stmt->supports_placeholders == PDO_PLACEHOLDER_NONE) {
    pdo_raise_impl_error(stmt->dbh, stmt, "HY093", "no parameters were bound");
    ret = -1;
    goto clean_up;
  }

  if (!params.empty() && bindno != params.size() &&
      stmt->supports_placeholders == PDO_PLACEHOLDER_NONE) {
    /* extra bit of validation for instances when same params are bound
       more then once */
    if (query_type != PDO_PLACEHOLDER_POSITIONAL && bindno > params.size()) {
      int ok = 1;
      for (plc = placeholders; plc; plc = plc->next) {
        if (!params.exists(String(plc->pos, plc->len, AttachLiteral))) {
          ok = 0;
          break;
        }
      }
      if (ok) {
        goto safe;
      }
    }
    pdo_raise_impl_error(stmt->dbh, stmt, "HY093",
                         "number of bound variables does not match number "
                         "of tokens");
    ret = -1;
    goto clean_up;
  }
safe:
  /* what are we going to do ? */
  if (stmt->supports_placeholders == PDO_PLACEHOLDER_NONE) {
    /* query generation */

    newbuffer_len = in.size();

    /* let's quote all the values */
    for (plc = placeholders; plc; plc = plc->next) {
      Variant vparam;
      if (query_type == PDO_PLACEHOLDER_POSITIONAL) {
        vparam = params[plc->bindno];
      } else {
        vparam = params[String(plc->pos, plc->len, AttachLiteral)];
      }
      if (vparam.isNull()) {
        /* parameter was not defined */
        ret = -1;
        pdo_raise_impl_error(stmt->dbh, stmt, "HY093",
                             "parameter was not defined");
        goto clean_up;
      }
      param = vparam.toObject().getTyped<PDOBoundParam>();
      if (stmt->dbh->support(PDOConnection::MethodQuoter)) {
        if (param->param_type == PDO_PARAM_LOB &&
            param->parameter.isResource()) {
          Variant buf = f_stream_get_contents(param->parameter);
          if (!same(buf, false)) {
            if (!stmt->dbh->quoter(buf.toString(), plc->quoted,
                                   param->param_type)) {
              /* bork */
              ret = -1;
              strcpy(stmt->error_code, stmt->dbh->error_code);
              goto clean_up;
            }
          } else {
            pdo_raise_impl_error(stmt->dbh, stmt, "HY105",
                                 "Expected a stream resource");
            ret = -1;
            goto clean_up;
          }
        } else {
          switch (param->parameter.getType()) {
          case KindOfNull:
            plc->quoted = "NULL";
            break;

          case KindOfByte:
          case KindOfInt16:
          case KindOfInt32:
          case KindOfInt64:
          case KindOfDouble:
            plc->quoted = param->parameter.toString();
            break;

          case KindOfBoolean:
            param->parameter = param->parameter.toInt64();
          default:
            if (!stmt->dbh->quoter(param->parameter.toString(), plc->quoted,
                                   param->param_type)) {
              /* bork */
              ret = -1;
              strcpy(stmt->error_code, stmt->dbh->error_code);
              goto clean_up;
            }
          }
        }
      } else {
        plc->quoted = param->parameter;
      }
      newbuffer_len += plc->quoted.size();
    }

rewrite:
    /* allocate output buffer */
    newbuffer = (char*)malloc(newbuffer_len + 1);
    newbuffer[newbuffer_len] = '\0';
    out = String(newbuffer, newbuffer_len, AttachString);

    /* and build the query */
    plc = placeholders;
    ptr = in.data();

    do {
      t = plc->pos - ptr;
      if (t) {
        memcpy(newbuffer, ptr, t);
        newbuffer += t;
      }
      memcpy(newbuffer, plc->quoted.data(), plc->quoted.size());
      newbuffer += plc->quoted.size();
      ptr = plc->pos + plc->len;

      plc = plc->next;
    } while (plc);

    t = (in.data() + in.size()) - ptr;
    if (t) {
      memcpy(newbuffer, ptr, t);
      newbuffer += t;
    }
    *newbuffer = '\0';
    out = out.substr(0, newbuffer - out.data());

    ret = 1;
    goto clean_up;

  } else if (query_type == PDO_PLACEHOLDER_POSITIONAL) {
    /* rewrite ? to :pdoX */
    StringBuffer idxbuf;
    const char *tmpl = stmt->named_rewrite_template ?
      stmt->named_rewrite_template : ":pdo%d";
    int bind_no = 1;

    newbuffer_len = in.size();

    for (plc = placeholders; plc; plc = plc->next) {
      int skip_map = 0;
      String name(plc->pos, plc->len, AttachLiteral);

      /* check if bound parameter is already available */
      if (!strcmp(name, "?") || !stmt->bound_param_map.exists(name)) {
        idxbuf.printf(tmpl, bind_no++);
      } else {
        idxbuf.clear();
        idxbuf.append(stmt->bound_param_map[name].toString());
        skip_map = 1;
      }

      plc->quoted = idxbuf.detach();
      newbuffer_len += plc->quoted.size();

      if (!skip_map && stmt->named_rewrite_template) {
        /* create a mapping */
        stmt->bound_param_map.set(name, plc->quoted);
      }

      /* map number to name */
      stmt->bound_param_map.set(plc->bindno, plc->quoted);
    }

    goto rewrite;

  } else {
    /* rewrite :name to ? */

    newbuffer_len = in.size();

    for (plc = placeholders; plc; plc = plc->next) {
      String name(plc->pos, plc->len, AttachLiteral);
      stmt->bound_param_map.set(plc->bindno, name);
      plc->quoted = "?";
    }

    goto rewrite;
  }

clean_up:

  while (placeholders) {
    plc = placeholders;
    placeholders = plc->next;
    free(plc);
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// PDOStatement

c_pdostatement::c_pdostatement() {
}

c_pdostatement::~c_pdostatement() {
  m_stmt.reset(); // needed for sweeping
}

void c_pdostatement::t___construct() {
  raise_error("You should not create a PDOStatement manually");
}

Variant c_pdostatement::t_execute(CArrRef params /* = null_array */) {
  strcpy(m_stmt->error_code, PDO_ERR_NONE);

  if (!params.empty()) {
    m_stmt->bound_params.reset();
    for (ArrayIter iter(params); iter; ++iter) {
      SmartObject<PDOBoundParam> param(new PDOBoundParam);
      param->param_type = PDO_PARAM_STR;
      param->parameter = iter.second();

      if (iter.first().isString()) {
        param->name = iter.first();
        param->paramno = -1;
      } else {
        int64 num_index = iter.first().toInt64();
        /* we're okay to be zero based here */
        if (num_index < 0) {
          pdo_raise_impl_error(m_stmt->dbh, m_stmt, "HY093", NULL);
          return false;
        }
        param->paramno = num_index;
      }

      if (!really_register_bound_param(param.get(), m_stmt, true)) {
        return false;
      }
    }
  }

  int ret = 1;
  if (PDO_PLACEHOLDER_NONE == m_stmt->supports_placeholders) {
    /* handle the emulated parameter binding, m_stmt->active_query_string
       holds the query with binds expanded and quoted. */
    ret = pdo_parse_params(m_stmt.get(), m_stmt->query_string,
                           m_stmt->active_query_string);
    if (ret == 0) { /* no changes were made */
      m_stmt->active_query_string = m_stmt->query_string;
    } else if (ret == -1) {
      /* something broke */
      PDO_HANDLE_STMT_ERR(m_stmt);
      return false;
    }
  } else if (!dispatch_param_event(m_stmt, PDO_PARAM_EVT_EXEC_PRE)) {
    PDO_HANDLE_STMT_ERR(m_stmt);
    return false;
  }
  if (m_stmt->executer()) {
    m_stmt->active_query_string.reset();
    if (!m_stmt->executed) {
      /* this is the first execute */

      if (m_stmt->dbh->alloc_own_columns && m_stmt->columns.empty()) {
        /* for "big boy" drivers, we need to allocate memory to fetch
         * the results into, so lets do that now */
        ret = pdo_stmt_describe_columns(m_stmt);
      }

      m_stmt->executed = 1;
    }

    if (ret && !dispatch_param_event(m_stmt, PDO_PARAM_EVT_EXEC_POST)) {
      return false;
    }

    return (bool)ret;
  }
  m_stmt->active_query_string.reset();
  PDO_HANDLE_STMT_ERR(m_stmt);
  return false;
}

Variant c_pdostatement::t_fetch(int64 how /* = q_pdo_FETCH_USE_DEFAULT */,
                                int64 orientation /* = q_pdo_FETCH_ORI_NEXT */,
                                int64 offset /* = 0 */) {
  strcpy(m_stmt->error_code, PDO_ERR_NONE);
  if (!pdo_stmt_verify_mode(m_stmt, how, false)) {
    return false;
  }

  Variant ret;
  if (!do_fetch(m_stmt, true, ret, (PDOFetchType)how,
                (PDOFetchOrientation)orientation, offset, NULL)) {
    PDO_HANDLE_STMT_ERR(m_stmt);
    return false;
  }
  return ret;
}

Variant c_pdostatement::t_fetchobject(CStrRef class_name /* = null_string */,
                                      CVarRef ctor_args /* = null */) {
  strcpy(m_stmt->error_code, PDO_ERR_NONE);
  if (!pdo_stmt_verify_mode(m_stmt, PDO_FETCH_CLASS, false)) {
    return false;
  }

  String old_clsname = m_stmt->fetch.clsname;
  Variant old_ctor_args = m_stmt->fetch.ctor_args;
  bool error = false;

  m_stmt->fetch.clsname = class_name;
  if (class_name.isNull()) {
    m_stmt->fetch.clsname = "stdclass";
  }
  if (!f_class_exists(m_stmt->fetch.clsname)) {
    pdo_raise_impl_error(m_stmt->dbh, m_stmt, "HY000",
                         "Could not find user-supplied class");
    error = true;
  }
  if (!ctor_args.isNull() && !ctor_args.isArray()) {
    pdo_raise_impl_error(m_stmt->dbh, m_stmt, "HY000",
                         "ctor_args must be either NULL or an array");
    error = true;
  }
  m_stmt->fetch.ctor_args = ctor_args;

  Variant ret;
  if (!error && !do_fetch(m_stmt, true, ret, PDO_FETCH_CLASS,
                          PDO_FETCH_ORI_NEXT, 0, NULL)) {
    error = true;
  }
  if (error) {
    PDO_HANDLE_STMT_ERR(m_stmt);
  }

  m_stmt->fetch.clsname = old_clsname;
  m_stmt->fetch.ctor_args = old_ctor_args;
  if (error) {
    return false;
  }
  return ret;
}

Variant c_pdostatement::t_fetchcolumn(int64 column_numner /* = 0 */) {
  strcpy(m_stmt->error_code, PDO_ERR_NONE);
  if (!do_fetch_common(m_stmt, PDO_FETCH_ORI_NEXT, 0, true)) {
    PDO_HANDLE_STMT_ERR(m_stmt);
    return false;
  }
  Variant ret;
  fetch_value(m_stmt, ret, column_numner, NULL);
  return ret;
}

Variant c_pdostatement::t_fetchall(int64 how /* = q_pdo_FETCH_USE_DEFAULT */,
                                   CVarRef class_name /* = null */,
                                   CVarRef ctor_args /* = null */) {
  if (!pdo_stmt_verify_mode(m_stmt, how, true)) {
    return false;
  }

  String old_clsname = m_stmt->fetch.clsname;
  Variant old_ctor_args = m_stmt->fetch.ctor_args;
  int error = 0;

  switch (how & ~PDO_FETCH_FLAGS) {
  case PDO_FETCH_CLASS:
    m_stmt->fetch.clsname = class_name;
    if (class_name.isNull()) {
      m_stmt->fetch.clsname = "stdclass";
    }
    if (!f_class_exists(m_stmt->fetch.clsname)) {
      pdo_raise_impl_error(m_stmt->dbh, m_stmt, "HY000",
                           "Could not find user-supplied class");
      error = 1;
    }
    if (!ctor_args.isNull() && !ctor_args.isArray()) {
      pdo_raise_impl_error(m_stmt->dbh, m_stmt, "HY000",
                           "ctor_args must be either NULL or an array");
      error = 1;
      break;
    }
    m_stmt->fetch.ctor_args = ctor_args;

    if (!error) {
      do_fetch_class_prepare(m_stmt);
    }
    break;

  case PDO_FETCH_FUNC:
    if (!f_function_exists(class_name)) {
      pdo_raise_impl_error(m_stmt->dbh, m_stmt, "HY000",
                           "no fetch function specified");
      error = 1;
    } else {
      m_stmt->fetch.func = class_name;
      do_fetch_func_prepare(m_stmt);
    }
    break;

  case PDO_FETCH_COLUMN:
    if (class_name.isNull()) {
      m_stmt->fetch.column = how & PDO_FETCH_GROUP ? -1 : 0;
    } else {
      m_stmt->fetch.column = class_name.toInt64();
    }
    if (!ctor_args.isNull()) {
      pdo_raise_impl_error(m_stmt->dbh, m_stmt, "HY000",
                           "Third parameter not allowed for "
                           "PDO::FETCH_COLUMN");
      error = 1;
    }
    break;
  }

  int flags = how & PDO_FETCH_FLAGS;

  if ((how & ~PDO_FETCH_FLAGS) == PDO_FETCH_USE_DEFAULT) {
    flags |= m_stmt->default_fetch_type & PDO_FETCH_FLAGS;
    how |= m_stmt->default_fetch_type & ~PDO_FETCH_FLAGS;
  }

  Variant *return_all = NULL;
  Variant return_value;
  Variant data;
  if (!error)  {
    strcpy(m_stmt->error_code, PDO_ERR_NONE);

    if ((how & PDO_FETCH_GROUP) || how == PDO_FETCH_KEY_PAIR ||
        (how == PDO_FETCH_USE_DEFAULT &&
         m_stmt->default_fetch_type == PDO_FETCH_KEY_PAIR)) {
      return_value = Array::Create();
      return_all = &return_value;
    }
    if (!do_fetch(m_stmt, true, data, (PDOFetchType)(how | flags),
                  PDO_FETCH_ORI_NEXT, 0, return_all)) {
      error = 2;
    }
  }
  if (!error) {
    if ((how & PDO_FETCH_GROUP)) {
      do {
        data.reset();
      } while (do_fetch(m_stmt, true, data, (PDOFetchType)(how | flags),
                        PDO_FETCH_ORI_NEXT, 0, return_all));
    } else if (how == PDO_FETCH_KEY_PAIR ||
               (how == PDO_FETCH_USE_DEFAULT &&
                m_stmt->default_fetch_type == PDO_FETCH_KEY_PAIR)) {
      while (do_fetch(m_stmt, true, data, (PDOFetchType)(how | flags),
                      PDO_FETCH_ORI_NEXT, 0, return_all));
    } else {
      return_value = Array::Create();
      do {
        return_value.append(data);
        data.reset();
      } while (do_fetch(m_stmt, true, data, (PDOFetchType)(how | flags),
                        PDO_FETCH_ORI_NEXT, 0, NULL));
    }
  }

  m_stmt->fetch.clsname = old_clsname;
  m_stmt->fetch.ctor_args = old_ctor_args;

  if (error) {
    PDO_HANDLE_STMT_ERR(m_stmt);
    if (error != 2) {
      return false;
    }

    /* on no results, return an empty array */
    if (!return_value.isArray()) {
      return_value = Array::Create();
    }
  }
  return return_value;
}

bool c_pdostatement::t_bindvalue(CVarRef paramno, CVarRef param,
                                 int64 type /* = q_pdo_PARAM_STR */) {
  return register_bound_param(paramno, param, type, 0, null, m_stmt, true);
}

bool c_pdostatement::t_bindparam(CVarRef paramno, Variant param,
                                 int64 type /* = q_pdo_PARAM_STR */,
                                 int64 max_value_len /* = 0 */,
                                 CVarRef driver_params /*= null */) {
  return register_bound_param(paramno, ref(param), type, max_value_len,
                              driver_params, m_stmt, true);
}

bool c_pdostatement::t_bindcolumn(CVarRef paramno, Variant param,
                                  int64 type /* = q_pdo_PARAM_STR */,
                                  int64 max_value_len /* = 0 */,
                                  CVarRef driver_params /* = null */) {
  return register_bound_param(paramno, ref(param), type, max_value_len,
                              driver_params, m_stmt, false);
}

int64 c_pdostatement::t_rowcount() {
  return m_stmt->row_count;
}

Variant c_pdostatement::t_errorcode() {
  if (m_stmt->error_code[0] == '\0') {
    return null;
  }
  return String(m_stmt->error_code, CopyString);
}

Array c_pdostatement::t_errorinfo() {
  Array ret;
  ret.append(String(m_stmt->error_code, CopyString));

  if (m_stmt->dbh->support(PDOConnection::MethodFetchErr)) {
    m_stmt->dbh->fetchErr(m_stmt.get(), ret);
  }

  int error_count = ret.size();
  int error_expected_count = 3;
  if (error_expected_count > error_count) {
    int error_count_diff = error_expected_count - error_count;
    for (int i = 0; i < error_count_diff; i++) {
      ret.append(null);
    }
  }
  return ret;
}

Variant c_pdostatement::t_setattribute(int64 attribute, CVarRef value) {
  if (!m_stmt->support(PDOStatement::MethodSetAttribute)) {
    pdo_raise_impl_error(m_stmt->dbh, m_stmt, "IM001",
                         "This driver doesn't support setting attributes");
    return false;
  }

  strcpy(m_stmt->error_code, PDO_ERR_NONE);
  if (m_stmt->setAttribute(attribute, value)) {
    return true;
  }
  PDO_HANDLE_STMT_ERR(m_stmt);
  return false;
}

Variant c_pdostatement::t_getattribute(int64 attribute) {
  Variant ret;
  if (!m_stmt->support(PDOStatement::MethodGetAttribute)) {
    if (!generic_stmt_attr_get(m_stmt, ret, attribute)) {
      pdo_raise_impl_error(m_stmt->dbh, m_stmt, "IM001",
                           "This driver doesn't support getting attributes");
      return false;
    }
    return ret;
  }

  strcpy(m_stmt->error_code, PDO_ERR_NONE);
  switch (m_stmt->getAttribute(attribute, ret)) {
  case -1:
    PDO_HANDLE_STMT_ERR(m_stmt);
    return false;
  case 0:
    if (!generic_stmt_attr_get(m_stmt, ret, attribute)) {
      /* XXX: should do something better here */
      pdo_raise_impl_error(m_stmt->dbh, m_stmt, "IM001",
                           "driver doesn't support getting that attribute");
      return false;
    }
    break;
  default:
    break;
  }
  return ret;
}

int64 c_pdostatement::t_columncount() {
  return m_stmt->column_count;
}

Variant c_pdostatement::t_getcolumnmeta(int64 column) {
  if (column < 0) {
    pdo_raise_impl_error(m_stmt->dbh, m_stmt, "42P10",
                         "column number must be non-negative");
    return false;
  }

  if (!m_stmt->support(PDOStatement::MethodGetColumnMeta)) {
    pdo_raise_impl_error(m_stmt->dbh, m_stmt, "IM001",
                         "driver doesn't support meta data");
    return false;
  }

  strcpy(m_stmt->error_code, PDO_ERR_NONE);
  Array ret;
  if (!m_stmt->getColumnMeta(column, ret)) {
    PDO_HANDLE_STMT_ERR(m_stmt);
    return false;
  }

  /* add stock items */
  PDOColumn *col = m_stmt->columns[column].toObject().getTyped<PDOColumn>();
  ret.set("name", col->name);
  ret.set("len", (int64)col->maxlen); /* FIXME: unsigned ? */
  ret.set("precision", (int64)col->precision);
  if (col->param_type != PDO_PARAM_ZVAL) {
    // if param_type is PDO_PARAM_ZVAL the driver has to provide correct data
    ret.set("pdo_type", (int64)col->param_type);
  }
  return ret;
}

bool c_pdostatement::t_setfetchmode(int _argc, int64 mode,
                                    CArrRef _argv /* = null_array */) {
  return pdo_stmt_set_fetch_mode(m_stmt, _argc, mode, _argv);
}

bool c_pdostatement::t_nextrowset() {
  if (!m_stmt->support(PDOStatement::MethodNextRowset)) {
    pdo_raise_impl_error(m_stmt->dbh, m_stmt, "IM001",
                         "driver does not support multiple rowsets");
    return false;
  }

  strcpy(m_stmt->error_code, PDO_ERR_NONE);

  /* un-describe */
  if (!m_stmt->columns.empty()) {
    m_stmt->columns.clear();
    m_stmt->column_count = 0;
  }

  if (!m_stmt->nextRowset()) {
    PDO_HANDLE_STMT_ERR(m_stmt);
    return false;
  }

  pdo_stmt_describe_columns(m_stmt);
  return true;
}

bool c_pdostatement::t_closecursor() {
  if (!m_stmt->support(PDOStatement::MethodCursorCloser)) {
    /* emulate it by fetching and discarding rows */
    do {
      while (m_stmt->fetcher(PDO_FETCH_ORI_NEXT, 0));
      if (!t_nextrowset()) {
        break;
      }
    } while (true);
    m_stmt->executed = 0;
    return true;
  }

  strcpy(m_stmt->error_code, PDO_ERR_NONE);
  if (!m_stmt->cursorCloser()) {
    PDO_HANDLE_STMT_ERR(m_stmt);
    return false;
  }
  m_stmt->executed = 0;
  return true;
}

Variant c_pdostatement::t_debugdumpparams() {
  Variant fobj = File::Open("php://output", "w");
  if (same(fobj, false)) {
    return false;
  }
  File *f = fobj.toObject().getTyped<File>();

  Array params;
  params.append(m_stmt->query_string.size());
  params.append(m_stmt->query_string.size());
  params.append(m_stmt->query_string.data());
  f->printf("SQL: [%d] %.*s\n", params);

  f->printf("Params:  %d\n", CREATE_VECTOR1(m_stmt->bound_params.size()));
  for (ArrayIter iter(m_stmt->bound_params); iter; ++iter) {
    if (iter.first().isString()) {
      String key = iter.first().toString();
      params = CREATE_VECTOR3(key.size(), key.size(), key.data());
      f->printf("Key: Name: [%d] %.*s\n", params);
    } else {
      f->printf("Key: Position #%ld:\n",
                CREATE_VECTOR1(iter.first().toInt64()));
    }

    PDOBoundParam *param = iter.second().toObject().getTyped<PDOBoundParam>();
    params.clear();
    params.append(param->paramno);
    params.append(param->name.size());
    params.append(param->name.size());
    params.append(param->name.data());
    params.append(param->is_param);
    params.append(param->param_type);
    f->printf("paramno=%d\nname=[%d] \"%.*s\"\nis_param=%d\nparam_type=%d\n",
              params);
  }
  return true;
}

Variant c_pdostatement::t___wakeup() {
  throw_pdo_exception(null, null, "You cannot serialize or unserialize "
                      "PDOStatement instances");
  return null;
}

Variant c_pdostatement::t___sleep() {
  throw_pdo_exception(null, null, "You cannot serialize or unserialize "
                      "PDOStatement instances");
  return null;
}

Variant c_pdostatement::t___destruct() {
  return null;
}

///////////////////////////////////////////////////////////////////////////////
// PDOException

c_pdoexception::c_pdoexception() {
}

c_pdoexception::~c_pdoexception() {
}

void c_pdoexception::t___construct() {
}

Variant c_pdoexception::t___destruct() {
  return null;
}

///////////////////////////////////////////////////////////////////////////////
}
