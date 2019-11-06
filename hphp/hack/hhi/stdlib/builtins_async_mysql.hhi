<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
namespace {
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

<<__PHPStdLib>>
function mysql_async_connect_start(string $server = "", string $username = "", string $password = "", string $database = "");
<<__PHPStdLib>>
function mysql_async_connect_completed($link_identifier);
<<__PHPStdLib>>
function mysql_async_query_start(string $query, $link_identifier);
<<__PHPStdLib>>
function mysql_async_query_result($link_identifier);
<<__PHPStdLib>>
function mysql_async_query_completed(resource $result);
<<__PHPStdLib>>
function mysql_async_fetch_array(resource $result, int $result_type = 1);
<<__PHPStdLib>>
function mysql_async_wait_actionable($items, float $timeout);
<<__PHPStdLib>>
function mysql_async_status($link_identifier);

class AsyncMysqlClient {
  public function __construct() { }
  static public function setPoolsConnectionLimit(int $limit) { }
  static public function connect(
      string $host,
      int $port,
      string $dbname,
      string $user,
      string $password,
      int $timeout_micros = -1,
      ?MySSLContextProvider $ssl_provider = null): Awaitable<AsyncMysqlConnection> { }

    static public function connectWithOpts(
      string $host,
      int $port,
      string $dbname,
      string $user,
      string $password,
      AsyncMysqlConnectionOptions $conn_opts): Awaitable<AsyncMysqlConnection> { }

    static public function connectAndQuery(
      Traversable<string> $queries,
      string $host,
      int $port,
      string $dbname,
      string $user,
      string $password,
      AsyncMysqlConnectionOptions $conn_opts,
      dict<string, string> $query_attributes = dict[],
    ): Awaitable<(AsyncMysqlConnectResult, Vector<AsyncMysqlQueryResult>)> { }

   static public function adoptConnection($connection) { }
}

class AsyncMysqlConnectionPool {
  public function __construct(darray $options) { }
  public function connect(string $host, int $port, string $dbname, string $user, string $password, int $timeout_micros = -1, string $caller = ""): Awaitable<AsyncMysqlConnection> { }
  public function connectWithOpts(
    string $host,
    int $port,
    string $dbname,
    string $user,
    string $password,
    AsyncMysqlConnectionOptions $conn_opts,
    string $caller = ""): Awaitable<AsyncMysqlConnection> { }

  public function getPoolStats(): darray { }
}

class MySSLContextProvider {
  private function __construct(): void { }
  public function isValid(): bool { }
}

class AsyncMysqlConnectionOptions {
  public function setConnectTimeout(int $timeout): void { }
  public function setConnectAttempts(int $attempts): void { }
  public function setTotalTimeout(int $timeout): void { }
  public function setQueryTimeout(int $timeout): void { }
  public function setConnectionAttributes(darray<string, string> $val): void { }
  public function setSSLOptionsProvider(?MySSLContextProvider $ssl_context): void { }
}

class AsyncMysqlClientStats {
  public function __construct() { }
  public function ioEventLoopMicrosAvg() : float {}
  public function callbackDelayMicrosAvg() : float {}
  public function ioThreadBusyMicrosAvg() : float {}
  public function ioThreadIdleMicrosAvg() : float {}
  public function notificationQueueSize() : int {}
}

class AsyncMysqlConnection {
  public function __construct() { }
  public function query(
    string $query,
    int $timeout_micros = -1,
    dict<string, string> $query_attributes = dict[],
  ): Awaitable<AsyncMysqlQueryResult>{ }
  public function queryf(HH\FormatString<HH\SQLFormatter> $query, ...$args): Awaitable<AsyncMysqlQueryResult>{ }
  public function queryAsync(\HH\Lib\SQL\Query $query): Awaitable<AsyncMysqlQueryResult>;
  public function multiQuery(
    Traversable<string> $query,
    int $timeout_micros = -1,
    dict<string, string> $query_attributes = dict[],
  ) { }
  public function escapeString(string $data): string { }
  public function close(): void { }
  public function releaseConnection() { }
  public function isValid(): bool { }
  public function serverInfo(): string { }
  public function sslSessionReused(): bool { }
  public function isSSL(): bool { }
  public function warningCount(): int { }
  public function host(): string { }
  public function port(): int { }
  public function setReusable(bool $reusable): void { }
  public function isReusable(): bool { }
  public function lastActivityTime(): float { }
  public function connectResult(): ?AsyncMysqlConnectResult { }
}

abstract class AsyncMysqlResult {
  public function __construct() { }
  public function elapsedMicros(): int { }
  public function startTime(): float { }
  public function endTime(): float { }

  public function clientStats() : AsyncMysqlClientStats { }
}

class AsyncMysqlConnectResult extends AsyncMysqlResult {
  public function __construct() { parent::__construct(); }
}

class AsyncMysqlErrorResult extends AsyncMysqlResult {
  public function __construct() { parent::__construct(); }
  public function mysql_errno(): int { }
  public function mysql_error(): string { }
  public function mysql_normalize_error(): string { }
  public function failureType(): string { }
}
class AsyncMysqlQueryErrorResult extends AsyncMysqlErrorResult {
  public function numSuccessfulQueries(): int { }
  public function getSuccessfulResults(): Vector<AsyncMysqlQueryResult> { }
}
class AsyncMysqlQueryResult extends AsyncMysqlResult {
  public function __construct() { parent::__construct();}
  public function numRowsAffected(): int { }
  public function lastInsertId(): int { }
  public function numRows(): int { }
  public function mapRows(): Vector<Map<string, ?string>> { }
  public function vectorRows(): Vector<KeyedContainer<int, ?string>> { }
  public function mapRowsTyped(): Vector<Map<string, mixed>> { }
  public function vectorRowsTyped(): Vector<KeyedContainer<int, mixed>> { }
 /* Can't put a return type for rowBlocks as it will ask that the type is
  * iterable because of the usage and then we can't have the AsyncMysqlRowBlock
  * implement the Iterable interface because mocks will complain they don't
  * implemplement the functions in the interface.
  **/
  public function rowBlocks() { }
  public function noIndexUsed() : bool { }
  public function recvGtid(): string { }
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
  // This should not be constructed from Hack source, but since the Exception
  // has a public constructor, we can not restrict the visibility here.
  public function __construct(private AsyncMysqlErrorResult $result) {}
  public function mysqlErrorCode(): int;
  public function mysqlErrorString(): string;
  public function timedOut(): bool;
  public function failed(): bool;
  public function getResult(): AsyncMysqlErrorResult;
}
class AsyncMysqlConnectException extends AsyncMysqlException {}
class AsyncMysqlQueryException extends AsyncMysqlException {}
}
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

namespace HH\Lib\SQL {
  interface ScalarFormat {
    public function format_f(?float $s): string;
    public function format_d(?int $int): string;
    public function format_s(?string $string): string;
  }

  interface ListFormat {
    // %LC - columns
    public function format_upcase_c(vec<string> $cols): string;
    // %Ls
    public function format_s(vec<string> $strs): string;
    // %Ld
    public function format_d(vec<int> $ints): string;
    // %Lf
    public function format_f(vec<float> $floats): string;

    /* INTENTIONALLY NOT IMPLEMENTED: %LO, %LA
     *
     * These are `dict<column, value>`; not added as the value
     * type must be `mixed`; use `%Q` instead to build queries in
     * a type-safe manner.
     */
  }

  interface QueryFormat extends ScalarFormat {
    // %%
    public function format_0x25(): string;

    // %T - table name
    public function format_upcase_t(string $s): string;
    // %C - column name
    public function format_upcase_c(string $s): string;
    // %K - SQL comment
    public function format_upcase_k(string $s): string;
    // %Q - subquery
    public function format_upcase_q(Query $q): string;

    // %L[sdfC] - lists
    public function format_upcase_l(): ListFormat;
    // %=[fds] - comparison
    public function format_0x3d(): ScalarFormat;

    /* INTENTIONALLY NOT IMPLEMEMENTED: %U, %W, %V, %m
     *
     * %U %W are `dict<column, value>`, and %V is
     * `vec<n-tuple(value, value...)>`, with `mixed` values. Use `%Q` instead
     * to build the query in a type-safe manner.
     *
     * %m is a straightforward `mixed` value, so also not implemented.
     */
  }

  type QueryFormatString = \HH\FormatString<QueryFormat>;

  final class Query {
    public function __construct(QueryFormatString $format, mixed ...$args) {}

    public function toString__FOR_DEBUGGING_ONLY(
      \AsyncMysqlConnection $conn
    ): string {}

    public function toUnescapedString__FOR_DEBUGGING_ONLY__UNSAFE(
    ): string {}
  }
}
