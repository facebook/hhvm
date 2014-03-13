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

#ifndef incl_HPHP_PDO_DRIVER_H_
#define incl_HPHP_PDO_DRIVER_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define PDO_DRIVER_API  20080721

enum PDOParamType {
  PDO_PARAM_NULL,

  /* int as in long (the php native int type).
   * If you mark a column as an int, PDO expects get_col to return
   * a pointer to a long */
  PDO_PARAM_INT,

  /* get_col ptr should point to start of the string buffer */
  PDO_PARAM_STR,

  /* get_col: when len is 0 ptr should point to a php_stream *,
   * otherwise it should behave like a string. Indicate a NULL field
   * value by setting the ptr to NULL */
  PDO_PARAM_LOB,

  /* get_col: will expect the ptr to point to a new PDOStatement object handle,
   * but this isn't wired up yet */
  PDO_PARAM_STMT, /* hierarchical result set */

  /* get_col ptr should point to a zend_bool */
  PDO_PARAM_BOOL,

  /* get_col ptr should point to a zval*
     and the driver is responsible for adding correct type information to
     get_column_meta() */
  PDO_PARAM_ZVAL
};

/* magic flag to denote a parameter as being input/output */
#define  PDO_PARAM_INPUT_OUTPUT   0x80000000

#define PDO_PARAM_FLAGS      0xFFFF0000

#define PDO_PARAM_TYPE(x)    ((x) & ~PDO_PARAM_FLAGS)

enum PDOFetchType {
  PDO_FETCH_USE_DEFAULT,
  PDO_FETCH_LAZY,
  PDO_FETCH_ASSOC,
  PDO_FETCH_NUM,
  PDO_FETCH_BOTH,
  PDO_FETCH_OBJ,
  PDO_FETCH_BOUND,  /* return true/false only; rely on bound columns */
  PDO_FETCH_COLUMN, /* fetch a numbered column only */
  PDO_FETCH_CLASS,  /* create an instance of named class, call ctor and
                       set properties */
  PDO_FETCH_INTO,   /* fetch row into an existing object */
  PDO_FETCH_FUNC,   /* fetch into function and return its result */
  PDO_FETCH_NAMED,  /* like PDO_FETCH_ASSOC, but can handle duplicate names */
  PDO_FETCH_KEY_PAIR,  /* fetch into an array where the 1st column is a key
                          and all subsequent columns are values */
  PDO_FETCH__MAX    /* must be last */
};

#define PDO_FETCH_FLAGS      0xFFFF0000  /* fetchAll() modes or'd to
                                            PDO_FETCH_XYZ */
#define PDO_FETCH_GROUP      0x00010000  /* fetch into groups */
#define PDO_FETCH_UNIQUE     0x00030000  /* fetch into groups assuming
                                            first col is unique */
#define PDO_FETCH_CLASSTYPE  0x00040000  /* fetch class gets its class
                                            name from 1st column */
#define PDO_FETCH_SERIALIZE  0x00080000  /* fetch class instances by
                                            calling serialize */
#define PDO_FETCH_PROPS_LATE 0x00100000  /* fetch props after calling ctor */

/* fetch orientation for scrollable cursors */
enum PDOFetchOrientation {
  PDO_FETCH_ORI_NEXT,  /* default: fetch the next available row */
  PDO_FETCH_ORI_PRIOR, /* scroll back to prior row and fetch that */
  PDO_FETCH_ORI_FIRST, /* scroll to the first row and fetch that */
  PDO_FETCH_ORI_LAST,  /* scroll to the last row and fetch that */
  PDO_FETCH_ORI_ABS,   /* scroll to an absolute numbered row and fetch that */
  PDO_FETCH_ORI_REL    /* scroll relative to the current row, and fetch that */
};

enum PDOAttributeType {
  PDO_ATTR_AUTOCOMMIT,          /* use to turn on or off auto-commit mode */
  PDO_ATTR_PREFETCH,            /* configure the prefetch size for drivers
                                   that support it. Size is in KB */
  PDO_ATTR_TIMEOUT,             /* connection timeout in seconds */
  PDO_ATTR_ERRMODE,             /* control how errors are handled */
  PDO_ATTR_SERVER_VERSION,      /* database server version */
  PDO_ATTR_CLIENT_VERSION,      /* client library version */
  PDO_ATTR_SERVER_INFO,         /* server information */
  PDO_ATTR_CONNECTION_STATUS,   /* connection status */
  PDO_ATTR_CASE,                /* control case folding for portability */
  PDO_ATTR_CURSOR_NAME,         /* name a cursor for use in
                                   "WHERE CURRENT OF <name>" */
  PDO_ATTR_CURSOR,              /* cursor type */
  PDO_ATTR_ORACLE_NULLS,        /* convert empty strings to NULL */
  PDO_ATTR_PERSISTENT,          /* pconnect style connection */
  PDO_ATTR_STATEMENT_CLASS,     /* array(classname, array(ctor_args)) to
                                   specify the class of the constructed
                                   statement */
  PDO_ATTR_FETCH_TABLE_NAMES,   /* include table names in the column names,
                                   where available */
  PDO_ATTR_FETCH_CATALOG_NAMES, /* include the catalog/db name names
                                   in the column names, where available */
  PDO_ATTR_DRIVER_NAME,         /* name of the driver (as used in the
                                   constructor) */
  PDO_ATTR_STRINGIFY_FETCHES,   /* converts integer/float types to strings
                                   during fetch */
  PDO_ATTR_MAX_COLUMN_LEN,      /* make database calculate maximum length
                                   of data found in a column */
  PDO_ATTR_DEFAULT_FETCH_MODE,  /* Set the default fetch mode */
  PDO_ATTR_EMULATE_PREPARES,    /* use query emulation rather than native */

  /* this defines the start of the range for driver specific options.
   * Drivers should define their own attribute constants beginning with this
   * value. */
  PDO_ATTR_DRIVER_SPECIFIC = 1000
};

enum PDOCursorType {
  PDO_CURSOR_FWDONLY,  /* forward only cursor (default) */
  PDO_CURSOR_SCROLL    /* scrollable cursor */
};

/* SQL-92 SQLSTATE error codes.

The character string value returned for an SQLSTATE consists of a two-character
class value followed by a three-character subclass value. A class value of 01
indicates a warning and is accompanied by a return code of
SQL_SUCCESS_WITH_INFO.

Class values other than '01', except for the class 'IM',
indicate an error and are accompanied by a return code of SQL_ERROR. The class
'IM' is specific to warnings and errors that derive from the implementation of
ODBC itself.

The subclass value '000' in any class indicates that there is no
subclass for that SQLSTATE. The assignment of class and subclass values is
defined by SQL-92.
*/

typedef char PDOErrorType[6]; /* SQLSTATE */

#define PDO_ERR_NONE  "00000"

enum PDOErrorMode {
  PDO_ERRMODE_SILENT,    /* just set error codes */
  PDO_ERRMODE_WARNING,   /* raise E_WARNING */
  PDO_ERRMODE_EXCEPTION  /* throw exceptions */
};

enum PDOCaseConversion {
  PDO_CASE_NATURAL,
  PDO_CASE_UPPER,
  PDO_CASE_LOWER
};

/* oracle interop settings */
enum PDONullHandling {
  PDO_NULL_NATURAL      = 0,
  PDO_NULL_EMPTY_STRING = 1,
  PDO_NULL_TO_STRING    = 2
};

/* hook for bound params */
enum PDOParamEvent {
  PDO_PARAM_EVT_ALLOC,
  PDO_PARAM_EVT_FREE,
  PDO_PARAM_EVT_EXEC_PRE,
  PDO_PARAM_EVT_EXEC_POST,
  PDO_PARAM_EVT_FETCH_PRE,
  PDO_PARAM_EVT_FETCH_POST,
  PDO_PARAM_EVT_NORMALIZE
};

enum PDOPlaceholderSupport {
  PDO_PLACEHOLDER_NONE       = 0,
  PDO_PLACEHOLDER_NAMED      = 1,
  PDO_PLACEHOLDER_POSITIONAL = 2
};

///////////////////////////////////////////////////////////////////////////////

class PDOConnection;
class PDODriver;
typedef hphp_const_char_map<PDODriver*> PDODriverMap;

class PDODriver {
public:
  static const PDODriverMap &GetDrivers() { return s_drivers;}

public:
  explicit PDODriver(const char *name);
  virtual ~PDODriver() {}

  const char *getName() const { return m_name;}

  PDOConnection *createConnection(const String& datasource, const String& username,
                                  const String& password, const Array& options);

private:
  static PDODriverMap s_drivers;

  const char *m_name;

  // Methods a driver needs to implement.
  virtual PDOConnection *createConnectionObject() = 0;
};

///////////////////////////////////////////////////////////////////////////////

class PDOStatement;
typedef SmartResource<PDOStatement> sp_PDOStatement;

/* represents a connection to a database */
class PDOConnection : public SweepableResourceData {
public:
  // This is special and doesn't use DECLARE_RESOURCE_ALLOCATION because it has
  // to live across requests. It is also the weirdest SweepableResourceData
  // as it can't use any PHP objects and deletes itself during sweep().
  static const char *PersistentKey;

  enum SupportedMethod {
    MethodCloser,
    MethodPreparer,
    MethodDoer,
    MethodQuoter,
    MethodBegin,
    MethodCommit,
    MethodRollback,
    MethodSetAttribute,
    MethodLastId,
    MethodFetchErr,
    MethodGetAttribute,
    MethodCheckLiveness,
    MethodPersistentShutdown
  };

public:
  PDOConnection();
  virtual ~PDOConnection();
  virtual bool create(const Array& options) = 0;
  virtual void sweep();

  CLASSNAME_IS("PDOConnection")
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  // alloc/release persistent storage.
  virtual void persistentSave();
  virtual void persistentRestore();

  virtual bool support(SupportedMethod method);

  /* close or otherwise disconnect the database */
  virtual bool closer();

  /* prepare a statement and stash driver specific portion into stmt */
  virtual bool preparer(const String& sql, sp_PDOStatement *stmt, const Variant& options);

  /* execute a statement (that does not return a result set) */
  virtual int64_t doer(const String& sql);

  /* quote a string */
  virtual bool quoter(const String& input, String &quoted, PDOParamType paramtype);

  /* transaction related */
  virtual bool begin();
  virtual bool commit();
  virtual bool rollback();

  /* setting of attributes */
  virtual bool setAttribute(int64_t attr, const Variant& value);

  /* return last insert id.  NULL indicates error condition, otherwise,
     the return value MUST be an emalloc'd NULL terminated string. */
  virtual String lastId(const char *name);

  /* fetch error information.  if stmt is not null, fetch information
     pertaining to the statement, otherwise fetch global error information.
     The driver should add the following information to the array "info" in
     this order:
     - native error code
     - string representation of the error code
     ... any other optional driver specific data ...  */
  virtual bool fetchErr(PDOStatement *stmt, Array &info);

  /* fetching of attributes: -1: error, 0: unsupported attribute */
  virtual int getAttribute(int64_t attr, Variant &value);

  /* checking/pinging persistent connections; return SUCCESS if the connection
     is still alive and ready to be used, FAILURE otherwise.
     You may set this handler to NULL, which is equivalent to returning
     SUCCESS. */
  virtual bool checkLiveness();

  /* called at request end for each persistent dbh; this gives the driver
     the opportunity to safely release resources that only have per-request
     scope */
  virtual void persistentShutdown();

public:
  /* credentials */
  std::string username;
  std::string password;

  /* if true, then data stored and pointed at by this handle must all be
   * persistently allocated */
  unsigned is_persistent:1;

  /* if true, driver should act as though a COMMIT were executed between
   * each executed statement; otherwise, COMMIT must be carried out manually
   * */
  unsigned auto_commit:1;

  /* if true, the handle has been closed and will not function anymore */
  unsigned is_closed:1;

  /* if true, the driver requires that memory be allocated explicitly for
   * the columns that are returned */
  unsigned alloc_own_columns:1;

  /* if true, commit or rollBack is allowed to be called */
  unsigned in_txn:1;

  /* max length a single character can become after correct quoting */
  unsigned max_escaped_char_length:3;

  /* oracle compat; see enum pdo_null_handling */
  unsigned oracle_nulls:2;

  /* when set, convert int/floats to strings */
  unsigned stringify:1;

  /* the sum of the number of bits here and the bit fields preceeding should
   * equal 32 */
  unsigned _reserved_flags:21;

  /* data source string used to open this handle */
  std::string data_source;

  /* the global error code. */
  PDOErrorType error_code;

  PDOErrorMode error_mode;

  PDOCaseConversion native_case, desired_case;

  /* persistent hash key associated with this handle */
  std::string persistent_id;

  PDODriver *driver;

  std::string def_stmt_clsname;

  std::string serialized_def_stmt_ctor_args;
  Variant def_stmt_ctor_args;

  /* when calling PDO::query(), we need to keep the error
   * context from the statement around until we next clear it.
   * This will allow us to report the correct error message
   * when PDO::query() fails */
  PDOStatement *query_stmt;

  /* defaults for fetches */
  PDOFetchType default_fetch_type;
};
typedef SmartResource<PDOConnection> sp_PDOConnection;

///////////////////////////////////////////////////////////////////////////////

/* describes a column */
class PDOColumn : public ResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(PDOColumn);
  PDOColumn();
  ~PDOColumn();

  CLASSNAME_IS("PDOColumn")
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

public:
  String name;
  unsigned long maxlen;
  PDOParamType param_type;
  unsigned long precision;
};

///////////////////////////////////////////////////////////////////////////////

/* describes a bound parameter */
class PDOBoundParam : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(PDOBoundParam);
  PDOBoundParam();
  ~PDOBoundParam();

  CLASSNAME_IS("PDOBoundParam")
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

public:
  int64_t paramno;           /* if -1, then it has a name, and we don't
                              know the index *yet* */
  String name;

  int64_t max_value_len;     /* as a hint for pre-allocation */

  Variant parameter;       /* the variable itself */
  PDOParamType param_type; /* desired or suggested type */

  Variant driver_params;   /* optional parameter(s) for the driver */

  PDOStatement *stmt;      /* for convenience in dtor */
  bool is_param;           /* parameter or column ? */

  void *driver_data;
};

///////////////////////////////////////////////////////////////////////////////

class c_pdo;
typedef SmartObject<c_pdo> sp_pdo;

/* represents a prepared statement */
class PDOStatement : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(PDOStatement);
  enum SupportedMethod {
    MethodExecuter,
    MethodFetcher,
    MethodDescriber,
    MethodGetColumn,
    MethodParamHook,
    MethodSetAttribute,
    MethodGetAttribute,
    MethodGetColumnMeta,
    MethodNextRowset,
    MethodCursorCloser
  };

public:
  PDOStatement();
  virtual ~PDOStatement();

  CLASSNAME_IS("PDOStatement")
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  virtual bool support(SupportedMethod method);

  /* start the query */
  virtual bool executer();

  /* causes the next row in the set to be fetched; indicates if there are no
   * more rows.  The ori and offset params modify which row should be returned,
   * if the stmt represents a scrollable cursor */
  virtual bool fetcher(PDOFetchOrientation ori, long offset);

  /* queries information about the type of a column, by index (0 based).
   * Driver should populate stmt->columns[colno] with appropriate info */
  virtual bool describer(int colno);

  /* retrieves pointer and size of the value for a column.
   * Note that PDO expects the driver to manage the lifetime of this data;
   * it will copy the value into a zval on behalf of the script.
   * If the driver sets caller_frees, ptr should point to emalloc'd memory
   * and PDO will free it as soon as it is done using it.
   */
  virtual bool getColumn(int colno, Variant &value);

  virtual bool paramHook(PDOBoundParam *param, PDOParamEvent event_type);

  /* setting of attributes */
  virtual bool setAttribute(int64_t attr, const Variant& value);

  /* fetching of attributes: -1: error, 0: unsupported attribute */
  virtual int getAttribute(int64_t attr, Variant &value);

  /* retrieves meta data for a numbered column.
   * Returns SUCCESS/FAILURE.
   * On SUCCESS, fill in return_value with an array with the following fields.
   * If a particular field is not supported, then the driver simply does not
   * add it to the array, so that scripts can use isset() to check for it.
   *
   * ### this is just a rough first cut, and subject to change ###
   *
   * these are added by PDO itself, based on data from the describe handler:
   *   name => the column name
   *   len => the length/size of the column
   *   precision => precision of the column
   *   pdo_type => an integer, one of the PDO_PARAM_XXX values
   *
   *   scale => the floating point scale
   *   table => the table for that column
   *   type => a string representation of the type, mapped to the PHP
   *           equivalent type name
   *   native_type => a string representation of the type, native style,
   *          if different from  the mapped name.
   *   flags => an array of flags including zero or more of the following:
   *            primary_key, not_null, unique_key, multiple_key, unsigned,
   *            auto_increment, blob
   *
   * Any driver specific data should be returned using a prefixed key or value.
   * Eg: custom data for the mysql driver would use either
   *   'mysql:foobar' => 'some data' // to add a new key to the array
   * or
   *   'flags' => array('not_null', 'mysql:some_flag'); // to add data to an
   *      existing key
   */
  virtual bool getColumnMeta(int64_t colno, Array &return_value);

  /* advances the statement to the next rowset of the batch.
   * If it returns 1, PDO will tear down its idea of columns
   * and meta data.  If it returns 0, PDO will indicate an error
   * to the caller. */
  virtual bool nextRowset();

  /* closes the active cursor on a statement, leaving the prepared
   * statement ready for re-execution.  Useful to explicitly state
   * that you are done with a given rowset, without having to explicitly
   * fetch all the rows. */
  virtual bool cursorCloser();

public:
  /* if true, we've already successfully executed this statement at least
   * once */
  unsigned executed:1;
  /* if true, the statement supports placeholders and can implement
   * bindParam() for its prepared statements, if false, PDO should
   * emulate prepare and bind on its behalf */
  unsigned supports_placeholders:2;

  unsigned _reserved:29;

  /* the number of columns in the result set; not valid until after
   * the statement has been executed at least once.  In some cases, might
   * not be valid until fetch (at the driver level) has been called at
   * least once.
   * */
  Array columns;
  long column_count;

  /* we want to keep the dbh alive while we live, so we own a reference */
  sp_PDOConnection dbh;

  /* keep track of bound input parameters.  Some drivers support
   * input/output parameters, but you can't rely on that working */
  Array bound_params;
  /* When rewriting from named to positional, this maps positions to names */
  Array bound_param_map;
  /* keep track of PHP variables bound to named (or positional) columns
   * in the result set */
  Array bound_columns;

  /* not always meaningful */
  long row_count;

  /* used to hold the statement's current query */
  String query_string;

  /* copy of the query with expanded binds ONLY for emulated-prepare drivers */
  String active_query_string;

  /* the cursor specific error code. */
  PDOErrorType error_code;

  /* for lazy fetches, we always return the same lazy object handle.
   * Let's keep it here. */
  Variant lazy_object_ref;

  /* defaults for fetches */
  PDOFetchType default_fetch_type;
  struct {
    int column;
    String clsname;
    String constructor;
    Variant ctor_args;
    String func;
    Variant fetch_args;
    Object object;
    Variant values;
    Variant into;
  } fetch;

  /* used by the query parser for driver specific
   * parameter naming (see pgsql driver for example) */
  const char *named_rewrite_template;
};

int pdo_parse_params(PDOStatement *stmt, const String& in, String &out);
void pdo_raise_impl_error(sp_PDOConnection dbh, sp_PDOStatement stmt,
                          const char *sqlstate, const char *supp);
void throw_pdo_exception(const Variant& code, const Variant& info,
                         const char *fmt, ...) ATTRIBUTE_PRINTF(3,4);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PDO_DRIVER_H_
