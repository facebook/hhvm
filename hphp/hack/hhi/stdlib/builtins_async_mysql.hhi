<?hh // decl
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

const int NOT_NULL_FLAG = 0;
const int PRI_KEY_FLAG = 0;
const int UNIQUE_KEY_FLAG = 0;
const int MULTIPLE_KEY_FLAG = 0;
const int UNSIGNED_FLAG = 0;
const int ZEROFILL_FLAG = 0;
const int BINARY_FLAG = 0;
const int AUTO_INCREMENT_FLAG = 0;
const int ENUM_FLAG = 0;
const int SET_FLAG = 0;
const int BLOB_FLAG = 0;
const int TIMESTAMP_FLAG = 0;
const int NUM_FLAG = 0;
const int NO_DEFAULT_VALUE_FLAG = 0;

const int MYSQL_TYPE_TINY = 0;
const int MYSQL_TYPE_SHORT = 0;
const int MYSQL_TYPE_LONG = 0;
const int MYSQL_TYPE_INT24 = 0;
const int MYSQL_TYPE_LONGLONG = 0;
const int MYSQL_TYPE_DECIMAL = 0;
const int MYSQL_TYPE_NEWDECIMAL = 0;
const int MYSQL_TYPE_FLOAT = 0;
const int MYSQL_TYPE_DOUBLE = 0;
const int MYSQL_TYPE_BIT = 0;
const int MYSQL_TYPE_TIMESTAMP = 0;
const int MYSQL_TYPE_DATE = 0;
const int MYSQL_TYPE_TIME = 0;
const int MYSQL_TYPE_DATETIME = 0;
const int MYSQL_TYPE_YEAR = 0;
const int MYSQL_TYPE_STRING = 0;
const int MYSQL_TYPE_VAR_STRING = 0;
const int MYSQL_TYPE_BLOB = 0;
const int MYSQL_TYPE_SET = 0;
const int MYSQL_TYPE_ENUM = 0;
const int MYSQL_TYPE_GEOMETRY = 0;
const int MYSQL_TYPE_NULL = 0;

const int ASYNC_OP_INVALID = 0;
const int ASYNC_OP_UNSET = 0;
const int ASYNC_OP_CONNECT = 0;
const int ASYNC_OP_QUERY = 0;

function mysql_async_connect_start($server = null, $username = null, $password = null, $database = null);
function mysql_async_connect_completed($link_identifier);
function mysql_async_query_start($query, $link_identifier);
function mysql_async_query_result($link_identifier);
function mysql_async_query_completed($result);
function mysql_async_fetch_array($result, $result_type = 1);
function mysql_async_wait_actionable($items, $timeout);
function mysql_async_status($link_identifier);

class AsyncMysqlClient {
  public function __construct() { }
  static public function setPoolsConnectionLimit(int $limit) { }
  static public function connect(string $host, int $port, string $dbname, string $user, string $password, int $timeout_micros = -1): Awaitable<AsyncMysqlConnection> { }
  static public function adoptConnection($connection) { }
}
class AsyncMysqlConnectionPool {
  public function __construct(array $options) { }
  public function connect(string $host, int $port, string $dbname, string $user, string $password, int $timeout_micros = -1, string $caller = "") { }
  public function getPoolStats(): array { }
}
class AsyncMysqlConnection {
  public function __construct() { }
  public function query(string $query, int $timeout_micros = -1): Awaitable<AsyncMysqlQueryResult>{ }
  public function queryf(HH\FormatString<HH\SQLFormatter> $query, ...$args): Awaitable<AsyncMysqlQueryResult>{ }
  public function multiQuery(Vector<string> $query, int $timeout_micros = -1) { }
  public function escapeString(string $data): string { }
  public function close(): void{ }
  public function releaseConnection() { }
  public function serverInfo() { }
  public function warningCount() { }
  public function host(): string{ }
  public function port(): int{ }
  public function setReusable(bool $reusable): void{ }
  public function isReusable(): bool { }
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
class AsyncMysqlQueryErrorResult extends AsyncMysqlErrorResult {
  public function numSuccessfulQueries(): int { }
  public function getSuccessfulResults(): Vector { }
}
class AsyncMysqlQueryResult extends AsyncMysqlResult {
  public function __construct() { parent::__construct();}
  public function numRowsAffected(): int { }
  public function lastInsertId(): int { }
  public function numRows(): int { }
  public function mapRows(): Vector<Map<string, string>>{ }
  public function vectorRows() { }
  public function mapRowsTyped(): Vector<Map<string, mixed>> { }
  public function vectorRowsTyped() { }
 /* Can't put a return type for rowBlocks as it will ask that the type is
  * iterable because of the usage and then we can't have the AsyncMysqlRowBlock
  * implement the Iterable interface because mocks will complain they don't
  * implemplement the functions in the interface.
  **/
  public function rowBlocks() { }
}
class AsyncMysqlRowBlock implements Countable, KeyedTraversable<int, AsyncMysqlRow> {
  public function __construct() { }
  public function at(int $row, mixed $field): mixed { }
  public function getFieldAsInt(int $row, mixed $field): int { }
  public function getFieldAsDouble(int $row, mixed $field): float { }
  public function getFieldAsString(int $row, mixed $field): string { }
  public function isNull(int $row, mixed $field): bool { }
  public function fieldType(mixed $field): int { }
  public function fieldFlags(mixed $field): int { }
  public function fieldName(int $field): string { }
  public function isEmpty(): bool { }
  public function fieldsCount(): int { }
  public function count(): int { }
  public function getIterator(): KeyedIterator<int, AsyncMysqlRow> { }
  public function getRow(int $row): AsyncMysqlRow { }
}
/* actually returned from AsyncMysqlRowBlock::getIterator
class AsyncMysqlRowBlockIterator implements Iterator, Traversable {
  public function __construct() { }
  public function valid() { }
  public function next() { }
  public function current() { }
  public function key() { }
  public function rewind() { }
}
*/
class AsyncMysqlRow implements MysqlRow {
  public function __construct() { }
  public function at(mixed $field): mixed { }
  public function getFieldAsInt(mixed $field): int { }
  public function getFieldAsDouble(mixed $field): float { }
  public function getFieldAsString(mixed $field): string { }
  public function isNull(mixed $field): bool { }
  public function fieldType(mixed $field): int { }
  public function count(): int { }
  public function getIterator(): KeyedIterator<string, mixed> { }
}
/* actually returned from AsyncMysqlRow::getIterator
class AsyncMysqlRowIterator implements KeyedIterator<string, mixed> {
  public function __construct() { }
  public function valid() { }
  public function next() { }
  public function current() { }
  public function key() { }
  public function rewind() { }
}
*/
interface MysqlRow extends Countable, KeyedTraversable<string, mixed>, IteratorAggregate<mixed>
{
  public function at(mixed $field): mixed;
  public function getFieldAsInt(mixed $field): int;
  public function getFieldAsDouble(mixed $field): float;
  public function getFieldAsString(mixed $field): string;
  public function fieldType(mixed $field): int;
  public function isNull(mixed $field): bool;
  public function count(): int;
  public function getIterator(): KeyedIterator<string, mixed>;
}
class AsyncMysqlException extends Exception {
  // Not actually protected, but no good comes of php code constructing these
  protected function __construct(private AsyncMysqlErrorResult $result) {}
  public function mysqlErrorCode(): int;
  public function mysqlErrorString(): string;
  public function timedOut(): bool;
  public function failed(): bool;
  public function getResult(): AsyncMysqlErrorResult;
}
class AsyncMysqlConnectException extends AsyncMysqlException {}
class AsyncMysqlQueryException extends AsyncMysqlException {}

namespace HH {
  interface SQLFormatter extends SQLScalarFormatter {
    public function format_0x25(): string; // %%
    public function format_0x3d(): SQLScalarFormatter; // %=[f,d,s] - comparison

    public function format_upcase_t(string $s): string; // table name
    public function format_upcase_c(string $s): string; // column name

    // %L[sdfC] - lists
    public function format_upcase_l(): SQLListFormatter;
  }

  interface SQLScalarFormatter {
    public function format_f(?float $s): string;
    public function format_d(?int $int): string;
    public function format_s(?string $string): string;
  }

  interface SQLListFormatter {
    public function format_upcase_c(\ConstVector<string> $cols): string; // %LC
    public function format_s(\ConstVector<string> $strs): string; // %Ls
    public function format_d(\ConstVector<int> $ints): string; // %Ld
    public function format_f(\ConstVector<float> $floats): string; // %Lf
  }
}
