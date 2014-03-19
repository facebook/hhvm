<?hh     // decl
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

define('NOT_NULL_FLAG', 0);
define('PRI_KEY_FLAG', 0);
define('UNIQUE_KEY_FLAG', 0);
define('MULTIPLE_KEY_FLAG', 0);
define('UNSIGNED_FLAG', 0);
define('ZEROFILL_FLAG', 0);
define('BINARY_FLAG', 0);
define('AUTO_INCREMENT_FLAG', 0);
define('ENUM_FLAG', 0);
define('SET_FLAG', 0);
define('BLOB_FLAG', 0);
define('TIMESTAMP_FLAG', 0);
define('NUM_FLAG', 0);
define('NO_DEFAULT_VALUE_FLAG', 0);
define('MYSQL_TYPE_TINY', 0);
define('MYSQL_TYPE_SHORT', 0);
define('MYSQL_TYPE_LONG', 0);
define('MYSQL_TYPE_INT24', 0);
define('MYSQL_TYPE_LONGLONG', 0);
define('MYSQL_TYPE_DECIMAL', 0);
define('MYSQL_TYPE_NEWDECIMAL', 0);
define('MYSQL_TYPE_FLOAT', 0);
define('MYSQL_TYPE_DOUBLE', 0);
define('MYSQL_TYPE_BIT', 0);
define('MYSQL_TYPE_TIMESTAMP', 0);
define('MYSQL_TYPE_DATE', 0);
define('MYSQL_TYPE_TIME', 0);
define('MYSQL_TYPE_DATETIME', 0);
define('MYSQL_TYPE_YEAR', 0);
define('MYSQL_TYPE_STRING', 0);
define('MYSQL_TYPE_VAR_STRING', 0);
define('MYSQL_TYPE_BLOB', 0);
define('MYSQL_TYPE_SET', 0);
define('MYSQL_TYPE_ENUM', 0);
define('MYSQL_TYPE_GEOMETRY', 0);
define('MYSQL_TYPE_NULL', 0);
class AsyncMysqlClient {
  public function __construct() { }
  static public function connect(string $host, int $port, string $dbname, string $user, string $password, int $timeout_micros = -1) { }
  static public function adoptConnection($connection) { }
}
class AsyncMysqlConnection {
  public function __construct() { }
  public function query(string $query, int $timeout_micros = -1) { }
  public function close() { }
  public function releaseConnection() { }
  public function serverInfo() { }
  public function host() { }
  public function port() { }
}
abstract class AsyncMysqlResult {
  public function __construct() { }
  public function elapsedMicros() { }
  public function startTime() { }
  public function endTime() { }
}
class AsyncMysqlErrorResult extends AsyncMysqlResult {
  public function __construct() { parent::__construct();}
  public function mysql_errno() { }
  public function mysql_error() { }
  public function failureType() { }
}
class AsyncMysqlQueryResult extends AsyncMysqlResult {
  public function __construct() { parent::__construct();}
  public function numRowsAffected(): int { }
  public function lastInsertId(): int { }
  public function mapRows() { }
  public function vectorRows() { }
  public function mapRowsTyped(): Vector<Map<string, ?mixed>> { }
  public function vectorRowsTyped() { }
 /* Can't put a return type for rowBlocks as it will ask that the type is
  * iterable because of the usage and then we can't have the AsyncMysqlRowBlock
  * implement the Iterable interface because mocks will complain they don't
  * implemplement the functions in the interface.
  **/
  public function rowBlocks() { }
}
class AsyncMysqlRowBlock {
  public function __construct() { }
  public function getFieldAsInt(int $row, mixed $field) { }
  public function getFieldAsDouble(int $row, mixed $field) { }
  public function getFieldAsString(int $row, mixed $field) { }
  public function isNull(int $row, mixed $field): bool { }
  public function fieldType(mixed $field): int { }
  public function fieldFlags(mixed $field): int { }
  public function fieldName(int $field): string { }
  public function isEmpty(): bool { }
  public function fieldsCount(): int { }
  public function count(): int { }
  public function getIterator(): AsyncMysqlRowBlockIterator { }
  public function getRow(int $row): AsyncMysqlRow { }
}
class AsyncMysqlRowBlockIterator {
  public function __construct() { }
  public function valid() { }
  public function next() { }
  public function current() { }
  public function key() { }
  public function rewind() { }
}
class AsyncMysqlRow implements MysqlRow {
  public function __construct() { }
  public function getFieldAsInt(mixed $field): int { }
  public function getFieldAsDouble(mixed $field): float { }
  public function getFieldAsString(mixed $field): string { }
  public function isNull(mixed $field): bool { }
  public function fieldType(mixed $field): int { }
  public function count(): int { }
  public function getIterator(): AsyncMysqlRowIterator { }
}
class AsyncMysqlRowIterator {
  public function __construct() { }
  public function valid() { }
  public function next() { }
  public function current() { }
  public function key() { }
  public function rewind() { }
}
