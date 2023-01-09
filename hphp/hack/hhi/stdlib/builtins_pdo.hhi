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
  const int PARAM_BOOL;
  const int PARAM_NULL;
  const int PARAM_INT;
  const int PARAM_STR;
  const int PARAM_LOB;
  const int PARAM_STMT;
  const int PARAM_INPUT_OUTPUT;
  const int PARAM_EVT_ALLOC;
  const int PARAM_EVT_FREE;
  const int PARAM_EVT_EXEC_PRE;
  const int PARAM_EVT_EXEC_POST;
  const int PARAM_EVT_FETCH_PRE;
  const int PARAM_EVT_FETCH_POST;
  const int PARAM_EVT_NORMALIZE;
  const int FETCH_USE_DEFAULT;
  const int FETCH_LAZY;
  const int FETCH_ASSOC;
  const int FETCH_NUM;
  const int FETCH_BOTH;
  const int FETCH_OBJ;
  const int FETCH_BOUND;
  const int FETCH_COLUMN;
  const int FETCH_CLASS;
  const int FETCH_INTO;
  const int FETCH_FUNC;
  const int FETCH_GROUP;
  const int FETCH_UNIQUE;
  const int FETCH_KEY_PAIR;
  const int FETCH_CLASSTYPE;
  const int FETCH_SERIALIZE;
  const int FETCH_PROPS_LATE;
  const int FETCH_NAMED;
  const int ATTR_AUTOCOMMIT;
  const int ATTR_PREFETCH;
  const int ATTR_TIMEOUT;
  const int ATTR_ERRMODE;
  const int ATTR_SERVER_VERSION;
  const int ATTR_CLIENT_VERSION;
  const int ATTR_SERVER_INFO;
  const int ATTR_CONNECTION_STATUS;
  const int ATTR_CASE;
  const int ATTR_CURSOR_NAME;
  const int ATTR_CURSOR;
  const int ATTR_ORACLE_NULLS;
  const int ATTR_PERSISTENT;
  const int ATTR_STATEMENT_CLASS;
  const int ATTR_FETCH_TABLE_NAMES;
  const int ATTR_FETCH_CATALOG_NAMES;
  const int ATTR_DRIVER_NAME;
  const int ATTR_STRINGIFY_FETCHES;
  const int ATTR_MAX_COLUMN_LEN;
  const int ATTR_EMULATE_PREPARES;
  const int ATTR_DEFAULT_FETCH_MODE;
  const int ERRMODE_SILENT;
  const int ERRMODE_WARNING;
  const int ERRMODE_EXCEPTION;
  const int CASE_NATURAL;
  const int CASE_LOWER;
  const int CASE_UPPER;
  const int NULL_NATURAL;
  const int NULL_EMPTY_STRING;
  const int NULL_TO_STRING;
  const int ERR_NONE;
  const int FETCH_ORI_NEXT;
  const int FETCH_ORI_PRIOR;
  const int FETCH_ORI_FIRST;
  const int FETCH_ORI_LAST;
  const int FETCH_ORI_ABS;
  const int FETCH_ORI_REL;
  const int CURSOR_FWDONLY;
  const int CURSOR_SCROLL;
  const int MYSQL_ATTR_USE_BUFFERED_QUERY;
  const int MYSQL_ATTR_LOCAL_INFILE;
  const int MYSQL_ATTR_MAX_BUFFER_SIZE;
  const int MYSQL_ATTR_INIT_COMMAND;
  const int MYSQL_ATTR_READ_DEFAULT_FILE;
  const int MYSQL_ATTR_READ_DEFAULT_GROUP;
  const int MYSQL_ATTR_COMPRESS;
  const int MYSQL_ATTR_DIRECT_QUERY;
  const int MYSQL_ATTR_FOUND_ROWS;
  const int MYSQL_ATTR_IGNORE_SPACE;
  const int MYSQL_ATTR_SSL_CA;
  const int MYSQL_ATTR_SSL_CAPATH;
  const int MYSQL_ATTR_SSL_CERT;
  const int MYSQL_ATTR_SSL_KEY;
  const int MYSQL_ATTR_SSL_CIPHER;
  const int HH_MYSQL_ATTR_READ_TIMEOUT;
  const int HH_MYSQL_ATTR_WRITE_TIMEOUT;
  public function __construct(
    string $dsn,
    string $username = "",
    string $password = "",
    HH\FIXME\MISSING_PARAM_TYPE $options = null,
  );
  public function prepare(
    string $statement,
    HH\FIXME\MISSING_PARAM_TYPE $options = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function beginTransaction(): HH\FIXME\MISSING_RETURN_TYPE;
  public function inTransaction(): HH\FIXME\MISSING_RETURN_TYPE;
  public function commit(): HH\FIXME\MISSING_RETURN_TYPE;
  public function rollBack(): HH\FIXME\MISSING_RETURN_TYPE;
  public function setAttribute(
    int $attribute,
    HH\FIXME\MISSING_PARAM_TYPE $value,
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
class PDOStatement /* implements Iterator<mixed> */ {
  public function __construct();
  public function execute(
    HH\FIXME\MISSING_PARAM_TYPE $params = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function fetch(
    int $how = 0,
    int $orientation = PDO::FETCH_ORI_NEXT,
    int $offset = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function fetchObject(
    string $class_name = "",
    HH\FIXME\MISSING_PARAM_TYPE $ctor_args = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function fetchColumn(
    int $column_numner = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function fetchAll(
    int $how = 0,
    HH\FIXME\MISSING_PARAM_TYPE $class_name = null,
    HH\FIXME\MISSING_PARAM_TYPE $ctor_args = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function bindValue(
    HH\FIXME\MISSING_PARAM_TYPE $paramno,
    HH\FIXME\MISSING_PARAM_TYPE $param,
    int $type = PDO::PARAM_STR,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function rowCount(): HH\FIXME\MISSING_RETURN_TYPE;
  public function errorCode(): HH\FIXME\MISSING_RETURN_TYPE;
  public function errorInfo(): HH\FIXME\MISSING_RETURN_TYPE;
  public function setAttribute(
    int $attribute,
    HH\FIXME\MISSING_PARAM_TYPE $value,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getAttribute(int $attribute): HH\FIXME\MISSING_RETURN_TYPE;
  public function columnCount(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getColumnMeta(int $column): HH\FIXME\MISSING_RETURN_TYPE;
  public function setFetchMode(
    int $mode,
    HH\FIXME\MISSING_PARAM_TYPE ...$args
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function nextRowset(): HH\FIXME\MISSING_RETURN_TYPE;
  public function closeCursor(): HH\FIXME\MISSING_RETURN_TYPE;
  public function debugDumpParams(): HH\FIXME\MISSING_RETURN_TYPE;
  public function current(): HH\FIXME\MISSING_RETURN_TYPE;
  public function key(): HH\FIXME\MISSING_RETURN_TYPE;
  public function next(): HH\FIXME\MISSING_RETURN_TYPE;
  public function rewind(): HH\FIXME\MISSING_RETURN_TYPE;
  public function valid(): bool;
  public function __wakeup()[]: HH\FIXME\MISSING_RETURN_TYPE;
  public function __sleep()[]: HH\FIXME\MISSING_RETURN_TYPE;
}
class PDOException extends RuntimeException {
  public ?varray<mixed> $errorInfo = null;
}
