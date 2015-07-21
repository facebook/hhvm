<?hh
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

/**
 * An asynchronous MySQL client
 *
 */
final class AsyncMysqlClient {

  /**
   * AsyncMysqlClient objects cannot be directly created.
   *
   */
  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Sets the limit of all pools using this client
   *
   * @param int $limit - The limit for all pools
   *
   */
  <<__HipHopSpecific, __Native>>
  public static function setPoolsConnectionLimit(int $limit): void;

  /**
   * Begin an async connection to a MySQL instance
   *
   * @param string $host - The hostname to connect to
   * @param int $port - The port to connect to
   * @param string $dbname - The initial database when connecting
   * @param string $user - The user to connect as
   * @param string $password - The password to connect with
   * @param int $timeout_micros - Timeout, in microseconds, for the connect;
   *   -1 for default, 0 for no timeout
   *
   */
  <<__HipHopSpecific, __Native>>
  public static function connect(string $host,
                                 int $port,
                                 string $dbname,
                                 string $user,
                                 string $password,
                                 int $timeout_micros = -1
                                ): ExternalThreadEventWaitHandle;

  /**
   * Create a new async connection from a synchronous MySQL instance
   *
   * @param mixed $connection - The synchronous MySQL connection.
   *
   */
  <<__HipHopSpecific, __Native>>
  public static function adoptConnection(mixed $connection
                                        ): AsyncMysqlConnection;
}

/**
 * An asynchronous MySQL connection pool
 *
 */
<<__NativeData("AsyncMysqlConnectionPool")>>
class AsyncMysqlConnectionPool {

  /**
   * @param array $pool_options - Options for pool
   *
   */
  <<__Native>>
  public function __construct(array $pool_options): void;

  /**
   * Returns the stats values for the pool
   *
   * @return array - Each position maps to a different stat value
   *
   */
  <<__HipHopSpecific, __Native>>
  public function getPoolStats(): array;

  /**
   * Begin an async connection to a MySQL instance
   *
   * @param string $host - The hostname to connect to
   * @param int $port - The port to connect to
   * @param string $dbname - The initial database when connecting
   * @param string $user - The user to connect as
   * @param string $password - The password to connect with
   * @param int $timeout_micros - Timeout, in microseconds, for the connect;
   *   -1 for default, 0 for no timeout
   * @param string $extra_key - Extra parameter to separate connections even
   *   better
   *
   */
  <<__HipHopSpecific, __Native>>
  public function connect(string $host,
                          int $port,
                          string $dbname,
                          string $user,
                          string $password,
                          int $timeout_micros = -1,
                          string $extra_key = ""
                         ): ExternalThreadEventWaitHandle;
}

/**
 * An active connection to a MySQL instance
 *
 */
<<__NativeData("AsyncMysqlConnection")>>
final class AsyncMysqlConnection {

  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Begin running a query; returns a WaitHandle
   *
   * @param string $query - The query itself
   * @param int $timeout_micros - Timeout, in microseconds, for the query to
   *   complete in; -1 for default, 0 for no timeout
   *
   */
  <<__HipHopSpecific, __Native>>
    function query(string $query,
                   int $timeout_micros = -1): ExternalThreadEventWaitHandle;

  /**
   * Execute a query with placeholders and parameters.
   *
   * For example:
   *   queryf("SELECT %C FROM %T WHERE %C %=s", $col1, $table, $col2, $value);
   * Supported placeholders:
   *   %T   table name
   *   %C   column name
   *   %s   nullable string (will be escaped)
   *   %d   integer
   *   %f   float
   *   %=s  nullable string comparison - expands to either:
   *          = 'escaped_string'
   *          IS NULL
   *   %=d  nullable integer comparison
   *   %=f  nullable float comparison
   *   %Q   raw SQL query. The typechecker intentionally does not recognize
   *          this, however, you can use it in combination with // UNSAFE
   *          if absolutely required
   */
  <<__HipHopSpecific, __Native>>
  function queryf(string $pattern,
                  ...$args): ExternalThreadEventWaitHandle;

  /**
   * Begin running a query with multiple statements; returns a WaitHandle
   *
   * @param array $queries - A vector of queries
   * @param int $timeout_micros - Timeout, in microseconds, for all queries to
   *   complete in; -1 for default, 0 for no timeout
   *
   */
  <<__HipHopSpecific, __Native>>
  function multiQuery(array $queries,
                      int $timeout_micros = -1): ExternalThreadEventWaitHandle;

  /**
   * Escape a string to be safe for including in a query.
   *
   * Equivalent to mysql_real_escape_string().
   */
  <<__HipHopSpecific, __Native>>
  function escapeString(string $data): string;

  /**
   * Close this connection
   *
   */
  <<__HipHopSpecific, __Native>>
  function close(): void;

  /**
   * Return a synchronous MySQL connection, destroying this connection in the
   *   process.
   *
   */
  <<__HipHopSpecific, __Native>>
  function releaseConnection(): mixed;

  /**
   * The server version of the server connected
   *
   */
  <<__HipHopSpecific, __Native>>
  function serverInfo(): string;

  /**
   * The number of errors, warnings, and notes returned during execution of
   *   the previous SQL statement
   *
   */
  <<__HipHopSpecific, __Native>>
  function warningCount(): int;

  /**
   * The host this connection is connected to
   *
   */
  <<__HipHopSpecific, __Native>>
  function host(): string;

  /**
   * The port this connection is connected to
   *
   */
  <<__HipHopSpecific, __Native>>
  function port(): int;

  /**
   * Sets if the connection can be recycled without any clean up
   *
   * @param bool $reusable - If it is reusable or not
   *
   */
  <<__HipHopSpecific, __Native>>
  function setReusable(bool $reusable): void;

  /**
   * Returns whether or not the connection is set as reusable by the pool
   *
   */
  <<__HipHopSpecific, __Native>>
  function isReusable(): bool;
}

/**
 * A base class for errors and query results.  This class contains timing
 *   information about a query or connection.
 *
 */
abstract class AsyncMysqlResult {

  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Time this operation took, in microseconds
   *
   */
  abstract function elapsedMicros(): int;

  /**
   * The time this operation began, in seconds since epoch
   *
   */
  abstract function startTime(): float;

  /**
   * The time this operation completed, in seconds since epoch
   *
   */
  abstract function endTime(): float;
}

/**
 * A class containing error information for a failed connect or query
 *
 */
<<__NativeData("AsyncMysqlErrorResult")>>
class AsyncMysqlErrorResult extends AsyncMysqlResult {

  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Time this operation took, in microseconds
   *
   */
  <<__HipHopSpecific, __Native>>
  function elapsedMicros(): int;

  /**
   * The time this operation began, in seconds since epoch
   *
   */
  <<__HipHopSpecific, __Native>>
  function startTime(): float;

  /**
   * The time this operation completed, in seconds since epoch
   *
   */
  <<__HipHopSpecific, __Native>>
  function endTime(): float;

  /**
   * The MySQL error number (see MySQL's errmsg.h for details, or the C API's
   *   mysql_errno() documentation)
   *
   */
  <<__HipHopSpecific, __Native>>
  function mysql_errno(): int;

  /**
   * A human-readable string for the error encountered
   *
   */
  <<__HipHopSpecific, __Native>>
  function mysql_error(): string;

  /**
   * The type of failure (either 'TimedOut', representing a timeout, or
   *   'Failed', representing the server rejecting our connection or query)
   *
   */
  <<__HipHopSpecific, __Native>>
  function failureType(): string;
}

/**
 * A class containing info about results for statements that ran before mysql
 *   error
 *
 */
<<__NativeData("AsyncMysqlQueryErrorResult")>>
final class AsyncMysqlQueryErrorResult extends AsyncMysqlErrorResult {

  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Returns the number of successfully executed queries
   *
   */
  <<__HipHopSpecific, __Native>>
  function numSuccessfulQueries(): int;

  /**
   * Returns the results that were fetched by the statements that succeeded
   *
   */
  <<__HipHopSpecific, __Native>>
  function getSuccessfulResults(): Vector;
}

/**
 * The result of a successfully executed query
 *
 */
<<__NativeData("AsyncMysqlQueryResult")>>
final class AsyncMysqlQueryResult extends AsyncMysqlResult {

  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Time this operation took, in microseconds
   *
   */
  <<__HipHopSpecific, __Native>>
  function elapsedMicros(): int;

  /**
   * The time this operation began, in seconds since epoch
   *
   */
  <<__HipHopSpecific, __Native>>
  function startTime(): float;

  /**
   * The time this operation completed, in seconds since epoch
   *
   */
  <<__HipHopSpecific, __Native>>
  function endTime(): float;

  /**
   * The number of rows affected (see the C API's mysql_num_rows()
   *   documentation)
   *
   */
  <<__HipHopSpecific, __Native>>
  function numRowsAffected(): int;

  /**
   * The last ID inserted, if one existed for this query (see the C API's
   *   mysql_insert_id() documentation)
   *
   */
  <<__HipHopSpecific, __Native>>
  function lastInsertId(): int;

  /**
   * The number of rows in this result
   *
   */
  <<__HipHopSpecific, __Native>>
  function numRows(): int;

  /**
   * The rows returned by this query, as a Vector of Map objects which map
   *   column names to possibly-null string values.
   *
   */
  <<__HipHopSpecific, __Native>>
  function mapRows(): Vector;

  /**
   * The rows returned by this query, as a Vector of Vector objects which hold
   *   the possibly-null string values of each column in the order of the
   *   original query.
   *
   */
  <<__HipHopSpecific, __Native>>
  function vectorRows(): Vector;

  /**
   * The rows returned by this query, as a Vector of Map objects which map
   *   column names to possibly-null mixed values.
   *
   */
  <<__HipHopSpecific, __Native>>
  function mapRowsTyped(): Vector;

  /**
   * The rows returned by this query, as a Vector of Vector objects which hold
   *   the possibly-null mixed values of each column in the order of the
   *   original query.
   *
   */
  <<__HipHopSpecific, __Native>>
  function vectorRowsTyped(): Vector;

  /**
   * Returns a Vector<AsyncMysqlRowBlock> representing all row blocks returned
   *   by this query.
   *
   */
  <<__HipHopSpecific, __Native>>
  function rowBlocks(): Vector;
}

/**
 * A class to represent a Row Block.
 *
 */
<<__NativeData("AsyncMysqlRowBlock")>>
final class AsyncMysqlRowBlock implements IteratorAggregate, Countable {

  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Get a field value
   *
   * @param int $row - the row index
   * @param mixed $field - the field index(int) or field name(string).
   *
   */
  <<__HipHopSpecific, __Native>>
  function at(int $row, mixed $field): mixed;

  /**
   * Get a certain field from a certain row as integer.
   *
   * @param int $row - the row index
   * @param mixed $field - the field index(int) or field name(string).
   *
   */
  <<__HipHopSpecific, __Native>>
  function getFieldAsInt(int $row, mixed $field): int;

  /**
   * Get a certain field from a certain row as double.
   *
   * @param int $row - the row index
   * @param mixed $field - the field index(int) or field name(string)
   *
   */
  <<__HipHopSpecific, __Native>>
  function getFieldAsDouble(int $row, mixed $field): float;

  /**
   * Get a certain field from a certain row as String.
   *
   * @param int $row - the row index
   *
   * @param mixed $field - the field index(int) or field name(string).
   *
   */
  <<__HipHopSpecific, __Native>>
  function getFieldAsString(int $row, mixed $field): string;

  /**
   * Returns true if a field is null.
   *
   * @param int $row - the row index
   *
   * @param mixed $field - the field index(int) or field name(string).
   *
   */
  <<__HipHopSpecific, __Native>>
  function isNull(int $row, mixed $field): bool;

  /**
   * The type of the field as a string.
   *
   * @param mixed $field - the field index(int) or field name(string).
   *
   */
  <<__HipHopSpecific, __Native>>
  function fieldType(mixed $field): int;

  /**
   * The flags of the field.
   *
   * @param mixed $field - the field index(int) or field name(string).
   *
   */
  <<__HipHopSpecific, __Native>>
  function fieldFlags(mixed $field): int;

  /**
   * The name of the field.
   *
   * @param int $field - the field index
   *
   */
  <<__HipHopSpecific, __Native>>
  function fieldName(int $field): string;

  /**
   * Returns true if no rows are returned.
   *
   */
  <<__HipHopSpecific, __Native>>
  function isEmpty(): bool;

  /**
   * The number of fields.
   *
   */
  <<__HipHopSpecific, __Native>>
  function fieldsCount(): int;

  /**
   * The number of rows.
   *
   */
  <<__HipHopSpecific, __Native>>
  function count(): int;

  /**
   * Get the iterator for the rows  in the block.
   *
   */
  <<__HipHopSpecific, __Native>>
  function getIterator(): AsyncMysqlRowBlockIterator;

  /**
   * Get a certain row.
   *
   * @param int $row - the row index
   *
   */
  <<__HipHopSpecific, __Native>>
  function getRow(int $row): AsyncMySqlRow;
}

/**
 * A class to represent an Iterator over the rows of a AsyncMysqlRowBlock.
 *
 */
<<__NativeData("AsyncMysqlRowBlockIterator")>>
final class AsyncMysqlRowBlockIterator implements HH\KeyedIterator {

  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Check if the current row number is valid
   *
   */
  <<__HipHopSpecific, __Native>>
  function valid(): bool;

  /**
   * Advance the iterator to the next row.
   *
   */
  <<__HipHopSpecific, __Native>>
  function next(): void;

  /**
   * Get the current row.
   *
   */
  <<__HipHopSpecific, __Native>>
  function current(): AsyncMysqlRow;

  /**
   * Get the current row number
   *
   */
  <<__HipHopSpecific, __Native>>
  function key(): int;

  /**
   * Reset the iterator to the first row.
   *
   */
  <<__HipHopSpecific, __Native>>
  function rewind(): void;
}

/**
 * A class to represent a Row.
 *
 */
<<__NativeData("AsyncMysqlRow")>>
final class AsyncMysqlRow implements MysqlRow {

  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Get field indexed by the `field`.
   *
   * @param mixed $field - the field index(int) or field name(string).
   *
   */
  <<__HipHopSpecific, __Native>>
  function at(mixed $field): mixed;

  /**
   * Get a certain field as integer.
   *
   * @param mixed $field - the field index(int) or field name(string).
   *
   */
  <<__HipHopSpecific, __Native>>
  function getFieldAsInt(mixed $field): int;

  /**
   * Get a certain field as double.
   *
   * @param mixed $field - the field index(int) or field name(string).
   *
   */
  <<__HipHopSpecific, __Native>>
  function getFieldAsDouble(mixed $field): float;

  /**
   * Get a certain field as String.
   *
   * @param mixed $field - the field index(int) or field name(string).
   *
   */
  <<__HipHopSpecific, __Native>>
  function getFieldAsString(mixed $field): string;

  /**
   * Returns true if a field is null.
   *
   * @param mixed $field - the field index(int) or field name(string).
   *
   */
  <<__HipHopSpecific, __Native>>
  function isNull(mixed $field): bool;

  /**
   * The type of the field as a string.
   *
   * @param mixed $field - the field index(int) or field name(string).
   *
   */
  <<__HipHopSpecific, __Native>>
  function fieldType(mixed $field): int;

  /**
   * Get the number of fields.
   *
   */
  <<__HipHopSpecific, __Native>>
  function count(): int;

  /**
   * Get the iterator over the fields in the row.
   *
   */
  <<__HipHopSpecific, __Native>>
  function getIterator(): AsyncMysqlRowIterator;
}

/**
 * A class to represent an Iterator over the fields in a row.
 *
 */
<<__NativeData("AsyncMysqlRowIterator")>>
final class AsyncMysqlRowIterator implements HH\KeyedIterator {

  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Check if the current field number is valid
   *
   */
  <<__HipHopSpecific, __Native>>
  function valid(): bool;

  /**
   * Advance the iterator to the next field.
   *
   */
  <<__HipHopSpecific, __Native>>
  function next(): void;

  /**
   * Get the current field.
   *
   */
  <<__HipHopSpecific, __Native>>
  function current(): string;

  /**
   * Get the current field number
   *
   */
  <<__HipHopSpecific, __Native>>
  function key(): int;

  /**
   * Reset the iterator to the first field.
   *
   */
  <<__HipHopSpecific, __Native>>
  function rewind(): void;
}
