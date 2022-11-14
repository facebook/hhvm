<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function pdo_drivers(): HH\FIXME\MISSING_RETURN_TYPE;
class PDO {
  const int PARAM_BOOL = 0;
  const int PARAM_NULL = 0;
  const int PARAM_INT = 0;
  const int PARAM_STR = 0;
  const int PARAM_LOB = 0;
  const int PARAM_STMT = 0;
  const int PARAM_INPUT_OUTPUT = 0;
  const int PARAM_EVT_ALLOC = 0;
  const int PARAM_EVT_FREE = 0;
  const int PARAM_EVT_EXEC_PRE = 0;
  const int PARAM_EVT_EXEC_POST = 0;
  const int PARAM_EVT_FETCH_PRE = 0;
  const int PARAM_EVT_FETCH_POST = 0;
  const int PARAM_EVT_NORMALIZE = 0;
  const int FETCH_USE_DEFAULT = 0;
  const int FETCH_LAZY = 0;
  const int FETCH_ASSOC = 0;
  const int FETCH_NUM = 0;
  const int FETCH_BOTH = 0;
  const int FETCH_OBJ = 0;
  const int FETCH_BOUND = 0;
  const int FETCH_COLUMN = 0;
  const int FETCH_CLASS = 0;
  const int FETCH_INTO = 0;
  const int FETCH_FUNC = 0;
  const int FETCH_GROUP = 0;
  const int FETCH_UNIQUE = 0;
  const int FETCH_KEY_PAIR = 0;
  const int FETCH_CLASSTYPE = 0;
  const int FETCH_SERIALIZE = 0;
  const int FETCH_PROPS_LATE = 0;
  const int FETCH_NAMED = 0;
  const int ATTR_AUTOCOMMIT = 0;
  const int ATTR_PREFETCH = 0;
  const int ATTR_TIMEOUT = 0;
  const int ATTR_ERRMODE = 0;
  const int ATTR_SERVER_VERSION = 0;
  const int ATTR_CLIENT_VERSION = 0;
  const int ATTR_SERVER_INFO = 0;
  const int ATTR_CONNECTION_STATUS = 0;
  const int ATTR_CASE = 0;
  const int ATTR_CURSOR_NAME = 0;
  const int ATTR_CURSOR = 0;
  const int ATTR_ORACLE_NULLS = 0;
  const int ATTR_PERSISTENT = 0;
  const int ATTR_STATEMENT_CLASS = 0;
  const int ATTR_FETCH_TABLE_NAMES = 0;
  const int ATTR_FETCH_CATALOG_NAMES = 0;
  const int ATTR_DRIVER_NAME = 0;
  const int ATTR_STRINGIFY_FETCHES = 0;
  const int ATTR_MAX_COLUMN_LEN = 0;
  const int ATTR_EMULATE_PREPARES = 0;
  const int ATTR_DEFAULT_FETCH_MODE = 0;
  const int ERRMODE_SILENT = 0;
  const int ERRMODE_WARNING = 0;
  const int ERRMODE_EXCEPTION = 0;
  const int CASE_NATURAL = 0;
  const int CASE_LOWER = 0;
  const int CASE_UPPER = 0;
  const int NULL_NATURAL = 0;
  const int NULL_EMPTY_STRING = 0;
  const int NULL_TO_STRING = 0;
  const int ERR_NONE = 0;
  const int FETCH_ORI_NEXT = 0;
  const int FETCH_ORI_PRIOR = 0;
  const int FETCH_ORI_FIRST = 0;
  const int FETCH_ORI_LAST = 0;
  const int FETCH_ORI_ABS = 0;
  const int FETCH_ORI_REL = 0;
  const int CURSOR_FWDONLY = 0;
  const int CURSOR_SCROLL = 0;
  const int MYSQL_ATTR_USE_BUFFERED_QUERY = 0;
  const int MYSQL_ATTR_LOCAL_INFILE = 0;
  const int MYSQL_ATTR_MAX_BUFFER_SIZE = 0;
  const int MYSQL_ATTR_INIT_COMMAND = 0;
  const int MYSQL_ATTR_READ_DEFAULT_FILE = 0;
  const int MYSQL_ATTR_READ_DEFAULT_GROUP = 0;
  const int MYSQL_ATTR_COMPRESS = 0;
  const int MYSQL_ATTR_DIRECT_QUERY = 0;
  const int MYSQL_ATTR_FOUND_ROWS = 0;
  const int MYSQL_ATTR_IGNORE_SPACE = 0;
  const int MYSQL_ATTR_SSL_CA = 0;
  const int MYSQL_ATTR_SSL_CAPATH = 0;
  const int MYSQL_ATTR_SSL_CERT = 0;
  const int MYSQL_ATTR_SSL_KEY = 0;
  const int MYSQL_ATTR_SSL_CIPHER = 0;
  const int HH_MYSQL_ATTR_READ_TIMEOUT = 0;
  const int HH_MYSQL_ATTR_WRITE_TIMEOUT = 0;
  public function __construct(
    string $dsn,
    string $username = "",
    string $password = "",
    $options = null,
  );
  public function prepare(
    string $statement,
    $options = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function beginTransaction(): HH\FIXME\MISSING_RETURN_TYPE;
  public function inTransaction(): HH\FIXME\MISSING_RETURN_TYPE;
  public function commit(): HH\FIXME\MISSING_RETURN_TYPE;
  public function rollBack(): HH\FIXME\MISSING_RETURN_TYPE;
  public function setAttribute(
    int $attribute,
    $value,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getAttribute(int $attribute): HH\FIXME\MISSING_RETURN_TYPE;
  public function exec(string $query): HH\FIXME\MISSING_RETURN_TYPE;
  public function lastInsertId(
    string $seqname = "",
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function errorCode(): HH\FIXME\MISSING_RETURN_TYPE;
  public function errorInfo(): HH\FIXME\MISSING_RETURN_TYPE;
  public function query(string $sql): HH\FIXME\MISSING_RETURN_TYPE;
  public function quote(
    string $str,
    int $paramtype = PDO::PARAM_STR,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function __wakeup()[]: HH\FIXME\MISSING_RETURN_TYPE;
  public function __sleep()[]: HH\FIXME\MISSING_RETURN_TYPE;
  static public function getAvailableDrivers(): HH\FIXME\MISSING_RETURN_TYPE;
}
class PDOStatement {
  public function __construct();
  public function execute($params = null): HH\FIXME\MISSING_RETURN_TYPE;
  public function fetch(
    int $how = 0,
    int $orientation = PDO::FETCH_ORI_NEXT,
    int $offset = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function fetchObject(
    string $class_name = "",
    $ctor_args = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function fetchColumn(
    int $column_numner = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function fetchAll(
    int $how = 0,
    $class_name = null,
    $ctor_args = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function bindValue(
    $paramno,
    $param,
    int $type = PDO::PARAM_STR,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function rowCount(): HH\FIXME\MISSING_RETURN_TYPE;
  public function errorCode(): HH\FIXME\MISSING_RETURN_TYPE;
  public function errorInfo(): HH\FIXME\MISSING_RETURN_TYPE;
  public function setAttribute(
    int $attribute,
    $value,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getAttribute(int $attribute): HH\FIXME\MISSING_RETURN_TYPE;
  public function columnCount(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getColumnMeta(int $column): HH\FIXME\MISSING_RETURN_TYPE;
  public function setFetchMode(
    int $mode,
    ...$args
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function nextRowset(): HH\FIXME\MISSING_RETURN_TYPE;
  public function closeCursor(): HH\FIXME\MISSING_RETURN_TYPE;
  public function debugDumpParams(): HH\FIXME\MISSING_RETURN_TYPE;
  public function current(): HH\FIXME\MISSING_RETURN_TYPE;
  public function key(): HH\FIXME\MISSING_RETURN_TYPE;
  public function next(): HH\FIXME\MISSING_RETURN_TYPE;
  public function rewind(): HH\FIXME\MISSING_RETURN_TYPE;
  public function valid(): HH\FIXME\MISSING_RETURN_TYPE;
  public function __wakeup()[]: HH\FIXME\MISSING_RETURN_TYPE;
  public function __sleep()[]: HH\FIXME\MISSING_RETURN_TYPE;
}
class PDOException extends RuntimeException {
  public ?varray $errorInfo = null;
}
