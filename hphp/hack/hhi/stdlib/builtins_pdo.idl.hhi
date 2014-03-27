<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function pdo_drivers() { }
class PDO {
  const PARAM_BOOL = 0;
  const PARAM_NULL = 0;
  const PARAM_INT = 0;
  const PARAM_STR = 0;
  const PARAM_LOB = 0;
  const PARAM_STMT = 0;
  const PARAM_INPUT_OUTPUT = 0;
  const PARAM_EVT_ALLOC = 0;
  const PARAM_EVT_FREE = 0;
  const PARAM_EVT_EXEC_PRE = 0;
  const PARAM_EVT_EXEC_POST = 0;
  const PARAM_EVT_FETCH_PRE = 0;
  const PARAM_EVT_FETCH_POST = 0;
  const PARAM_EVT_NORMALIZE = 0;
  const FETCH_USE_DEFAULT = 0;
  const FETCH_LAZY = 0;
  const FETCH_ASSOC = 0;
  const FETCH_NUM = 0;
  const FETCH_BOTH = 0;
  const FETCH_OBJ = 0;
  const FETCH_BOUND = 0;
  const FETCH_COLUMN = 0;
  const FETCH_CLASS = 0;
  const FETCH_INTO = 0;
  const FETCH_FUNC = 0;
  const FETCH_GROUP = 0;
  const FETCH_UNIQUE = 0;
  const FETCH_KEY_PAIR = 0;
  const FETCH_CLASSTYPE = 0;
  const FETCH_SERIALIZE = 0;
  const FETCH_PROPS_LATE = 0;
  const FETCH_NAMED = 0;
  const ATTR_AUTOCOMMIT = 0;
  const ATTR_PREFETCH = 0;
  const ATTR_TIMEOUT = 0;
  const ATTR_ERRMODE = 0;
  const ATTR_SERVER_VERSION = 0;
  const ATTR_CLIENT_VERSION = 0;
  const ATTR_SERVER_INFO = 0;
  const ATTR_CONNECTION_STATUS = 0;
  const ATTR_CASE = 0;
  const ATTR_CURSOR_NAME = 0;
  const ATTR_CURSOR = 0;
  const ATTR_ORACLE_NULLS = 0;
  const ATTR_PERSISTENT = 0;
  const ATTR_STATEMENT_CLASS = 0;
  const ATTR_FETCH_TABLE_NAMES = 0;
  const ATTR_FETCH_CATALOG_NAMES = 0;
  const ATTR_DRIVER_NAME = 0;
  const ATTR_STRINGIFY_FETCHES = 0;
  const ATTR_MAX_COLUMN_LEN = 0;
  const ATTR_EMULATE_PREPARES = 0;
  const ATTR_DEFAULT_FETCH_MODE = 0;
  const ERRMODE_SILENT = 0;
  const ERRMODE_WARNING = 0;
  const ERRMODE_EXCEPTION = 0;
  const CASE_NATURAL = 0;
  const CASE_LOWER = 0;
  const CASE_UPPER = 0;
  const NULL_NATURAL = 0;
  const NULL_EMPTY_STRING = 0;
  const NULL_TO_STRING = 0;
  const ERR_NONE = 0;
  const FETCH_ORI_NEXT = 0;
  const FETCH_ORI_PRIOR = 0;
  const FETCH_ORI_FIRST = 0;
  const FETCH_ORI_LAST = 0;
  const FETCH_ORI_ABS = 0;
  const FETCH_ORI_REL = 0;
  const CURSOR_FWDONLY = 0;
  const CURSOR_SCROLL = 0;
  const MYSQL_ATTR_USE_BUFFERED_QUERY = 0;
  const MYSQL_ATTR_LOCAL_INFILE = 0;
  const MYSQL_ATTR_MAX_BUFFER_SIZE = 0;
  const MYSQL_ATTR_INIT_COMMAND = 0;
  const MYSQL_ATTR_READ_DEFAULT_FILE = 0;
  const MYSQL_ATTR_READ_DEFAULT_GROUP = 0;
  const MYSQL_ATTR_COMPRESS = 0;
  const MYSQL_ATTR_DIRECT_QUERY = 0;
  const MYSQL_ATTR_FOUND_ROWS = 0;
  const MYSQL_ATTR_IGNORE_SPACE = 0;
  public function __construct($dsn, $username = null, $password = null, $options = null) { }
  public function prepare($statement, $options = null) { }
  public function beginTransaction() { }
  public function commit() { }
  public function rollBack() { }
  public function setAttribute($attribute, $value) { }
  public function getAttribute($attribute) { }
  public function exec($query) { }
  public function lastInsertId($seqname = null) { }
  public function errorCode() { }
  public function errorInfo() { }
  public function query($sql) { }
  public function quote($str, $paramtype = null) { }
  public function __wakeup() { }
  public function __sleep() { }
  static public function getAvailableDrivers() { }
}
class PDOStatement {
  public function __construct() { }
  public function execute($params = null) { }
  public function fetch($how = 0, $orientation = null, $offset = 0) { }
  public function fetchObject($class_name = null, $ctor_args = null) { }
  public function fetchColumn($column_numner = 0) { }
  public function fetchAll($how = 0, $class_name = null, $ctor_args = null) { }
  public function bindValue($paramno, $param, $type = null) { }
  public function bindParam($paramno, &$param, $type = null, $max_value_len = 0, $driver_params = null) { }
  public function bindColumn($paramno, &$param, $type = null, $max_value_len = 0, $driver_params = null) { }
  public function rowCount() { }
  public function errorCode() { }
  public function errorInfo() { }
  public function setAttribute($attribute, $value) { }
  public function getAttribute($attribute) { }
  public function columnCount() { }
  public function getColumnMeta($column) { }
  public function setFetchMode($mode, ...) { }
  public function nextRowset() { }
  public function closeCursor() { }
  public function debugDumpParams() { }
  public function current() { }
  public function key() { }
  public function next() { }
  public function rewind() { }
  public function valid() { }
  public function __wakeup() { }
  public function __sleep() { }
}
