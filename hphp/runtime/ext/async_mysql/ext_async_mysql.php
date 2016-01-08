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
 * An asynchronous MySQL client.
 *
 * This class allows you to asynchronously connect to a MySQL client. You
 * can directly connect to the MySQL client with the `connect()` method; in
 * addition you can use this class in conjunction with
 * `AsyncMysqlConnectionPool` pools by setting the limit of connections on
 * any given pool, and using `AsyncMysqlConnectionPool::connect()`.
 *
 * There is some duplication with this class. If possible, you should directly
 * construct connection pools via `new AsyncMysqlConnectionPool()` and then
 * call `AsyncMysqlConnectionPool::connect()` to connect to the MySQL client
 * using those pools. Here we optionally set pool limits and call `connect()`
 * on this class. `AsyncMysqlConnectionPool` provides more flexibility with
 * other available options, etc.
 *
 * In fact, there is discussion about deprecating `AsyncMysqlClient` all
 * together to avoid having this choice. But, for now, you can use this class
 * for asynchronous connection(s) to a MySQL database.
 *
 * @guide /hack/async/extensions
 * @guide /hack/async/introduction
 */
final class AsyncMysqlClient {

  /**
   * @internal
   *
   * AsyncMysqlClient objects cannot be directly created.
   *
   */
  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Sets the connection limit of all connection pools using this client.
   *
   * Use this function to toggle the number of allowed async connections on the
   * pools connecting to MySQL with this current client. For example, if you
   * set the limit to 2, and you try a third connection on the same pool, an
   * `AsyncMysqlConnectException` exception will be thrown.
   *
   * @param $limit - The limit for all pools.
   *
   */
  <<__HipHopSpecific, __Native>>
  public static function setPoolsConnectionLimit(int $limit): void;

  /**
   * Begin an async connection to a MySQL instance.
   *
   * Use this to asynchronously connect to a MySQL instance.
   *
   * Normally you would use this to make one asynchronous connection to the
   * MySQL client.
   *
   * If you want to be able to pool up a bunch of connections, you would call
   * `setPoolsConnectionLimit()`, create a default pool of connections with
   * `AsyncMysqlConnectionPool()::__construct()`, which now
   * has that limit set, and then call `AsyncMysqlConnectionPool()::connect()`.
   *
   * @param $host - The hostname to connect to.
   * @param $port - The port to connect to.
   * @param $dbname - The initial database to use when connecting.
   * @param $user - The user to connect as.
   * @param $password - The password to connect with.
   * @param $timeout_micros - Timeout, in microseconds, for the connect; -1 for
   *                          default, 0 for no timeout.
   * @param $ssl_context - Optionally allow the connection to tunnel via SSL.
   *
   * @return - an `Awaitable` representing an `AsyncMysqlConnection`. `await`
   * or `join` this result to obtain the actual connection.
   */
  <<__HipHopSpecific, __Native>>
  public static function connect(string $host,
                                 int $port,
                                 string $dbname,
                                 string $user,
                                 string $password,
                                 int $timeout_micros = -1,
                                 ?MySSLContextProvider $ssl_context = null,
                                ): Awaitable<AsyncMysqlConnection>;

  /**
   * Create a new async connection from a synchronous MySQL instance.
   *
   * This is a synchronous function. You will block until the connection has
   * been adopted to an `AsyncMysqlConnection`. Then you will be able to use
   * the async `AsyncMysqlConnection` methods like `queryf()`, etc.
   *
   * This is a tricky function to use and we are actually thinking of
   * deprecating it. This function *requrires* a deprecated, MySQL resource.
   * Once this resource is adpoted by a call to this function, it is no longer
   * valid in the context on which it was being used.
   *
   * If you are using this function, you might consider just creating a
   * connection pool via `AsyncMysqlConnectionPool` since you presumably have
   * all the connection details anyway.
   *
   * @param $connection - The synchronous MySQL connection.
   *
   * @return - An `AsyncMysqlConnection` instance.
   */
  <<__HipHopSpecific, __Native>>
  public static function adoptConnection(mixed $connection
                                        ): AsyncMysqlConnection;
}

/**
 * An asynchronous MySQL connection pool.
 *
 * This class provides a mechanism to create a pool of connections to a MySQL
 * client that can be utilized and reused as needed.
 *
 * When a client requests a connection from the pool, it may get one that
 * already exists; this avoids the overhead of establishing a new connection.
 *
 * This is the *highly recommended* way to create connections to a MySQL
 * client, as opposed to using the `AsyncMysqlClient` class which does not give
 * you nearly the flexibility. In fact, there is discussion about deprecating
 * the `AsyncMysqlClient` class all together.
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 */
<<__NativeData("AsyncMysqlConnectionPool")>>
class AsyncMysqlConnectionPool {

  /**
   * Create a pool of connections to access a MySQL client.
   *
   * You can pass this constructor an `array` of options to tweak the behavior
   * of your pool. If you don't want an options, pass an empty `array()`.
   *
   * Here are the keys for that array, and all values are `int`, except for
   * `expiration_policy`, which is a `string`:
   *
   * - `per_key_connection_limit`: The maximum number of connections allowed
   *                               in the pool for a single combination of
   *                               hostname, port, db and username. The default
   *                               is 50.
   * - `pool_connection_limit`: The maximum number of connections allowed in
   *                            the pool. The default is 5000. It is
   *                            interesting to note that this is the option
   *                            that is set when you call
   *                            `AsyncMysqlClient::setPoolsConnectionLimit()`.
   * - `idle_timeout_micros`: The maximum amount of time, in microseconds, that
   *                          a connection is allowed to sit idle in the pool
   *                          becore being destroyed. The default is 4 seconds.
   * - `age_timeout_micros`: The maximum age, in microseconds, that a connection
   *                         in the pool will be allowed to reach before being
   *                         destroyed. The default is 60 seconds.
   * - `expiration_policy`: A `string` of either `'IdleTime'` or `'Age'" that
   *                        specifies whehter connections in the pool will be
   *                        destroyed based on how long it sits idle or total
   *                        age in the pool. The default is `'Age'`.
   *
   * @param $pool_options - The `array` of options for the connection pool.
   *                        The key to each array element is an option listed
   *                        above, while the value is an `int` or `string`,
   *                        depending on the option.
   */
  <<__Native>>
  public function __construct(array $pool_options): void;

  /**
   * Returns statistical information for the current pool.
   *
   * Information provided includes the number of pool connections that were
   * created and destroyed, how many connections were requested, and how many
   * times the pool was hit or missed when creating the connection. The
   * returned `array` keys are:
   *
   * - `created_pool_connections`
   * - `destroyed_pool_connections`
   * - `connections_requested`
   * - `pool_hits`
   * - `pool_misses`
   *
   * @return - A string-keyed `array` with the statistical information above.
   */
  <<__HipHopSpecific, __Native>>
  public function getPoolStats(): array;

  /**
   * Begin an async connection to a MySQL instance.
   *
   * Once you have your pool constructed, you use this method to connect to the
   * MySQL client. The connection pool will either create a new connection or
   * use one of the recently available connections from the pool itself.
   *
   * @param $host - The hostname to connect to.
   * @param $port - The port to connect to.
   * @param $dbname - The initial database to use when connecting.
   * @param $user - The user to connect as.
   * @param $password - The password to connect with.
   * @param $timeout_micros - Timeout, in microseconds, for the connect; -1
   *                          for default, 0 for no timeout.
   * @param $extra_key - An extra parameter to help the internal connection
   *                     pool infrastructure separate connections even better.
   *                     Usually, the default `""` is fine.
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
                         ): Awaitable<AsyncMysqlConnection>;
}

/**
 * An active connection to a MySQL instance.
 *
 * When you call `connect()` with a connection provided by the pool established
 * with `AsyncMysqlConnectionPool`, you are returned this connection object to
 * actual do real work with the MySQL database, with the primary work being
 * querying the database itself.
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 */
<<__NativeData("AsyncMysqlConnection")>>
final class AsyncMysqlConnection {

  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Begin running a query on the MySQL database client.
   *
   * If you have a direct query that requires no placeholders, then you can
   * use this method. It takes a raw string query that will be executed as-is.
   *
   * You may want to call `escapeString()` to ensure that any queries out of
   * your direct control are safe.
   *
   * @param $query - The query itself.
   * @param $timeout_micros - The maximum time, in microseconds, in which the
   *                          query must be completed; -1 for default, 0 for
   *                          no timeout.
   *
   * @return - an `Awaitable` representing the result of your query. Use
   *           `await` or `join` to get the actual `AsyncMysqlQueryResult`
   *           object.
   */
  <<__HipHopSpecific, __Native>>
    function query(string $query,
                   int $timeout_micros = -1): Awaitable<AsyncMysqlQueryResult>;

  /**
   * Execute a query with placeholders and parameters.
   *
   * This is probably the more common of the two query methods, given its
   * flexibility and automatic escaping in most string cases.
   *
   * For example:
   *   `queryf("SELECT %C FROM %T WHERE %C %=s", $col1, $table, $col2, $value);`
   *
   * The supported placeholders are:
   *  - `%T`   table name
   *  - `%C`   column name
   *  - `%s`   nullable string (will be escaped)
   *  - `%d`   integer
   *  - `%f`   float
   *  - `%=s`  nullable string comparison - expands to either:
   *             `= 'escaped_string'`
   *             `IS NULL`
   *  - `%=d`  nullable integer comparison
   *  - `%=f`  nullable float comparison
   *  - `%Q`   raw SQL query. The typechecker intentionally does not recognize
   *           this, however, you can use it in combination with // UNSAFE
   *           if absolutely required. Use this at your own risk as it could
   *           open you up for SQL injection.
   *  - `%Lx`  where `x` is one of `T`, `C`, `s`, `d`, or `f`, represents a list
   *           of table names, column names, nullable strings, integers or
   *           floats, respectively. Pass a `Vector` of values to have it
   *           expanded into a comma-separated list. Parentheses are not
   *           added automatically around the placeholder in the query string,
   *           so be sure to add them if necessary.
   *
   * With the exception of `%Q`, any strings provided will be properly
   * escaped.
   *
   * @param $pattern - The query string with any placeholders.
   * @param $args - The real values for all of the placeholders in your query
   *                string. You must have as many values as you do
   *                placeholders.
   *
   * @return - an `Awaitable` representing the result of your query. Use
   *           `await` or `join` to get the actual `AsyncMysqlQueryResult`
   *           object.
   */
  <<__HipHopSpecific, __Native>>
  function queryf(string $pattern,
                  ...$args): Awaitable<AsyncMysqlQueryResult>;

  /**
   * Begin running a query with multiple statements.
   *
   * `AsyncMysqlConnection::multiQuery()` is similar to
   * `AsyncMysqlConnection::query()`, except that you can pass an array of
   * `string` queries to run one after the other. Then when you `await` or
   * `join` on the returned `Awaitable`, you will get a `Vector` of
   * `AsyncMysqlQueryResult`, one result for each query.
   *
   * @param $queries - A `Vector` of queries, with each query being a `string`
   *                    in the array.
   * @param $timeout_micros - The maximum time, in microseconds, in which the
   *                          query must be completed; -1 for default, 0 for
   *                          no timeout.
   *
   * @return - an `Awaitable` representing the result of your mutli-query. Use
   *           `await` or `join` to get the actual `Vector` of
   *           `AsyncMysqlQueryResult` objects.
   */
  <<__HipHopSpecific, __Native>>
  function multiQuery(
    array $queries,
    int $timeout_micros = -1): Awaitable<Vector<AsyncMysqlQueryResult>>;

  /**
   * Escape a string to be safe to include in a query.
   *
   * Use this method to ensure your query is safe from, for example, SQL
   * injection.
   *
   * This method is equivalent to PHP's
   * [mysql_real_escape_string()](http://goo.gl/bnxqtE).
   *
   * @param $data - The string to properly escape.
   *
   * @return - The escaped string.
   */
  <<__HipHopSpecific, __Native>>
  function escapeString(string $data): string;

  /**
   * Close the current connection.
   */
  <<__HipHopSpecific, __Native>>
  function close(): void;

  /**
   * Releases the current connection and returns a synchronous MySQL connection.
   *
   * This method will destroy the current `AsyncMysqlConnection` object and give
   * you back a vanilla, synchronous MySQL resource.
   *
   * @return - A `resouce` respresenting a
   *           [MySQL](http://php.net/manual/en/book.mysql.php) resource, or
   *           `false` on failure.
   */
  <<__HipHopSpecific, __Native>>
  function releaseConnection(): mixed;

  /**
   * The MySQL server version associated with the current connection.
   *
   * @return - The server version as a `string`.
   */
  <<__HipHopSpecific, __Native>>
  function serverInfo(): string;

  /**
   * Returns whether or not the current connection reused the SSL session
   * from another SSL connection. The session is set by MySSLContextProvider.
   * Some cases, the server can deny the session that was set and the handshake
   * will create a new one, in those cases this function will return `false`.
   * If this connections isn't SSL, `false` will be returned as well.
   *
   * @return - `true` if this is a SSL connection and the SSL session was
   *           reused; `false` otherwise.
   */
  <<__HipHopSpecific, __Native>>
  function sslSessionReused(): bool;


  /**
   * Returns whether or not the current connection was established as SSL based
   * on client flag exchanged during handshake.
   *
   * @return - `true` if this is a SSL connection; `false` otherwise
   */
  <<__HipHopSpecific, __Native>>
  function isSSL(): bool;

  /**
   * The number of errors, warnings, and notes returned during execution of
   * the previous SQL statement.
   *
   * @return - The `int` count of errors, warnings, etc.
   */
  <<__HipHopSpecific, __Native>>
  function warningCount(): int;

  /**
   * The hostname associated with the current connection.
   *
   * @return - The hostname as a `string`.
   */
  <<__HipHopSpecific, __Native>>
  function host(): string;

  /**
   * The port on which the MySQL instance is running.
   *
   * @return - The port as an `int`.
   */
  <<__HipHopSpecific, __Native>>
  function port(): int;

  /**
   * Sets if the current connection can be recycled without any clean up.
   *
   * By default, the current connection *is* resuable.
   *
   * If a connection in a `AsyncMysqlConnectionPool` is used, but you call
   * `setReusable(false)`, then you will have to create a whole new connection
   * instead of reusing this particular connection.
   *
   * @param $reusable - Pass `true` to make the connection reusable; `false`
   *                    otherwise.
   */
  <<__HipHopSpecific, __Native>>
  function setReusable(bool $reusable): void;

  /**
   * Returns whether or not the current connection is reusable.
   *
   * By default, the current connection is reusable by the pool. But if you call
   * `setResuable(false)`, then the current connection will not be reusable by
   * the connection pool.
   *
   * @return - `true` if the connection is reusable; `false` otherwise.
   */
  <<__HipHopSpecific, __Native>>
  function isReusable(): bool;

  /**
   * Last time a successful activity was made in the current connection, in
   * seconds since epoch.
   *
   * The first successful activity of the current connection is its creation.
   *
   * @return - A `float` representing the the number of seconds ago since epoch
   *           that we had successful activity on the current connection.
   */
  <<__HipHopSpecific, __Native>>
  function lastActivityTime(): float;

  /**
   * Returns the `AsyncMysqlConnectResult` for the current connection.
   *
   * An `AsyncMysqlConnectResult` provides information about the timing for
   * creating the current connection.
   *
   * @return - An `AsyncMysqlConnectResult` object or `null` if the
   *           `AsyncMysqlConnection` was not created in the MySQL client.
   */
  <<__HipHopSpecific, __Native>>
  function connectResult(): ?AsyncMysqlConnectResult;
}

/**
 * This class holds the SSL Context Provider that MySQL client will use to
 * establish a SSL connection.
 *
 * While not required, you can pass a `MySSLContextProvider` to
 * `AsyncMysqlClient::connect()`.
 *
 * The SSL context data isn't accessible by PHP; it is for internal use only.
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 */
<<__NativeData("MySSLContextProvider")>>
class MySSLContextProvider {
  /**
   * @internal
   */
  private function __contruct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Determines if the current SSL Context Provider is valid.
   *
   * @return - `true` if the provider is valid; `false` otherwise.
   */
  <<__HipHopSpecific, __Native>>
  public function isValid(): bool;
}

/**
 * Provides timing statistics about the MySQL client.
 *
 * This class provides round-trip and callback timing information for various
 * operations on the MySQL client.
 *
 * This information can be used to know how the performance of the MySQL client
 * may have affected a given result.
 *
 * For example, if you have a `AsyncMysqlConnection`, you can call:
 *
 * `$conn->connectResult()->clientStats()->ioEventLoopMicrosAvg()`
 *
 * to get round-trip timing information on the connection event itself.
 *
 * Basically any concrete implementation of `AsyncMysqlResult` can provide
 * these type of statistics by calling its `clientStats()` method and a method
 * on this class.
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 */
<<__NativeData("AsyncMysqlClientStats")>>
class AsyncMysqlClientStats {
  /**
   * @internal
   *
   * The statistics should be retrieved in the EventBase thread; they are
   * appended to the result of an operation so the performance state can be
   * logged.
   */
  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Average loop time of the MySQL client event, in microseconds.
   *
   * An event can include a connection, an error condition, a query, etc.
   *
   * This returns an exponentially-smoothed average.
   *
   * @return - A `float` representing the average for an event to happen on this
   *           MySQL client.
   */
  <<__HipHopSpecific, __Native>>
  function ioEventLoopMicrosAvg() : float;

  /**
   * Average delay between when a callback is scheduled in the MySQL client
   * and when it's actually ran, in microseconds.
   *
   * The callback can be from creating a connection, inducing an error
   * condition, executing a query, etc.
   *
   * This returns an exponentially-smoothed average.
   *
   * @return - A `float` representing the average callback dealy on this
   *           MySQL client.
   */
  <<__HipHopSpecific, __Native>>
  function callbackDelayMicrosAvg() : float;
}

/**
 * A base class for connection, query and error results.
 *
 * This class is `abstract` and cannot be instantiated, but provides the methods
 * that concrete classes must implement, which are timing information methods
 * regarding a query, connection or a resulting error.
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 */
<<__NativeData("AsyncMysqlResult")>>
abstract class AsyncMysqlResult {

  /**
   * @internal
   *
   * Abstract class
   */
  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * The total time for the specific MySQL operation, in microseconds.
   *
   * @return - the total operation time as `int` microseconds.
   */
  abstract function elapsedMicros(): int;

  /**
   * The start time for the specific MySQL operation, in seconds since epoch.
   *
   * @return - the start time as `float` seconds since epoch.
   */
  abstract function startTime(): float;

  /**
   * The end time for the specific MySQL operation, in seconds since epoch.
   *
   * @return - the end time as `float` seconds since epoch.
   */
  abstract function endTime(): float;

  /**
   * Returns the MySQL client statistics at the moment the result was created.
   *
   * This information can be used to know how the performance of the MySQL
   * client may have affected the result.
   *
   * @return - an `AsyncMysqlClientStats` object to query about event and
   *           callback timing to the MySQL client for the specific result.
   */
  abstract function clientStats(): AsyncMysqlClientStats;

}

/**
 * Provides the result information for when the connection to the MySQL
 * client is made successfully.
 *
 * This class is instantiated through a call from the connection object
 * via `AsyncMysqlConnection::connectResult()`.
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 */
<<__NativeData("AsyncMysqlConnectResult")>>
final class AsyncMysqlConnectResult extends AsyncMysqlResult {

  /**
   * @internal
   */
  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * The total time for the establishment of the MySQL connection,
   * in microseconds.
   *
   * @return - the total establishing connection time as `int` microseconds.
   */
  <<__HipHopSpecific, __Native>>
  function elapsedMicros(): int;

  /**
   * The start time for the connection operation, in seconds since epoch.
   *
   * @return - the start time as `float` seconds since epoch.
   */
  <<__HipHopSpecific, __Native>>
  function startTime(): float;

  /**
   * The end time of the connection operation, in seconds since epoch.
   *
   * @return - the end time as `float` seconds since epoch.
   */
  <<__HipHopSpecific, __Native>>
  function endTime(): float;

  /**
   * Returns the MySQL client statistics at the moment the connection was
   * established.
   *
   * This information can be used to know how the performance of the
   * MySQL client may have affected the connecting operation.
   *
   * @return - an `AsyncMysqlClientStats` object to query about event and
   *           callback timing to the MySQL client for the connection.
   */
  <<__HipHopSpecific, __Native>>
  function clientStats(): AsyncMysqlClientStats;

}

/**
 * Contains error information for a failed operation (e.g., connection, query).
 *
 * This class is instantiated when an `AsyncMysqlException` is thrown and
 * `AsyncMysqlException::getResult()` is called.
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 */
<<__NativeData("AsyncMysqlErrorResult")>>
class AsyncMysqlErrorResult extends AsyncMysqlResult {

  /**
   * @internal
   */
  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * The total time for the MySQL error condition to occur, in microseconds.
   *
   * @return - the total error producing time as `int` microseconds.
   */
  <<__HipHopSpecific, __Native>>
  function elapsedMicros(): int;

  /**
   * The start time when the error was produced, in seconds since epoch.
   *
   * @return - the start time as `float` seconds since epoch.
   */
  <<__HipHopSpecific, __Native>>
  function startTime(): float;

  /**
   * The end time when the error was produced, in seconds since epoch.
   *
   * @return - the end time as `float` seconds since epoch.
   */
  <<__HipHopSpecific, __Native>>
  function endTime(): float;

  /**
   * Returns the MySQL client statistics for the events that produced the error.
   *
   * This information can be used to know how the performance of the
   * MySQL client may have affected the operation that produced the error.
   *
   * @return - an `AsyncMysqlClientStats` object to query about event and
   *           callback timing to the MySQL client for whatever caused the
   *            error.
   */
  <<__HipHopSpecific, __Native>>
  function clientStats(): AsyncMysqlClientStats;

  /**
   * Returns the MySQL error number for this result.
   *
   * See MySQL's
   * [mysql_errno()](http://dev.mysql.com/doc/refman/5.0/en/mysql-errno.html)
   * for information on the error numbers.
   *
   * @return - The error number as an `int`.
   */
  <<__HipHopSpecific, __Native>>
  function mysql_errno(): int;

  /**
   * Returns a human-readable string for the error encountered in this result.
   *
   * @return - The error string.
   */
  <<__HipHopSpecific, __Native>>
  function mysql_error(): string;

  /**
   * Returns an alternative, normalized version of the error message provided by
   * mysql_error().
   *
   * Sometimes the message is the same, depending on if there was an explicit
   * normalized string provided by the MySQL client.
   *
   * @return - The normalized error string.
   */
  <<__HipHopSpecific, __Native>>
  function mysql_normalize_error(): string;

  /**
   * The type of failure that produced this result.
   *
   * The string returned will be either `'TimedOut'`, representing a timeout, or
   * `'Failed'`, representing the server rejecting the connection or query.
   *
   * @return - the type of failure, either `'TimedOut'` or `'Failed'`.
   */
  <<__HipHopSpecific, __Native>>
  function failureType(): string;
}

/**
 * Contains the information about results for query statements that ran before
 * a MySQL error.
 *
 * This class is instantiated when an `AsyncMysqlQueryException` is thrown and
 * `AsyncMysqlQueryException::getResult()` is called.
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 */
<<__NativeData("AsyncMysqlQueryErrorResult")>>
final class AsyncMysqlQueryErrorResult extends AsyncMysqlErrorResult {

  /**
   * @internal
   */
  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Returns the number of successfully executed queries.
   *
   * If there were any successful queries before receiving the error, this will
   * let you know how many of those there were.
   *
   * @return - The number of successful queries before the error as an `int`.
   */
  <<__HipHopSpecific, __Native>>
  function numSuccessfulQueries(): int;

  /**
   * Returns the results that were fetched by the successful query statements.
   *
   * @return - A `Vector` of `AsyncMysqlQueryResult` objects for each result
   *           produced by a successful query statement.
   */
  <<__HipHopSpecific, __Native>>
  function getSuccessfulResults(): Vector<AsyncMysqlQueryResult>;
}

/**
 * The result of a successfully executed MySQL query.
 *
 * Not only does this class provide timing information about retrieving the
 * successful result, it provides the actual result information (e.g., result
 * rows).
 *
 * You get an `AsyncMysqlQueryResult` through calls to
 * `AsyncMysqlConnection::query()`, `AsyncMysqlConection::queryf()` and
 * `AsyncMysqlConnection::multiQuery()`
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 */
<<__NativeData("AsyncMysqlQueryResult")>>
final class AsyncMysqlQueryResult extends AsyncMysqlResult {

  /**
   * @internal
   */
  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * The total time for the successful query to occur, in microseconds.
   *
   * @return - the total successful result producing time as `int` microseconds.
   */
  <<__HipHopSpecific, __Native>>
  function elapsedMicros(): int;

  /**
   * The start time when the successful query began, in seconds since epoch.
   *
   * @return - the start time as `float` seconds since epoch.
   */
  <<__HipHopSpecific, __Native>>
  function startTime(): float;

  /**
   * The end time when the successful query began, in seconds since epoch.
   *
   * @return - the end time as `float` seconds since epoch.
   */
  <<__HipHopSpecific, __Native>>
  function endTime(): float;

  /**
   * Returns the MySQL client statistics at the moment the successful query
   * ended.
   *
   * This information can be used to know how the performance of the
   * MySQL client may have affected the query operation.
   *
   * @return - an `AsyncMysqlClientStats` object to query about event and
   *           callback timing to the MySQL client for the query.
   */
  <<__HipHopSpecific, __Native>>
  function clientStats(): AsyncMysqlClientStats;

  /**
   * The number of database rows affected in the current result.
   *
   * This is particularly useful for `INSERT`, `DELETE`, `UPDATE` statements.
   *
   * This is complementary to `numRows()` as they might be the same value, but
   * if this was an `INSERT` query, for example, then this might be a non-zero
   * value, while `numRows()` would be 0.
   *
   * See the MySQL's [mysql_affected_rows()](http://goo.gl/1Sj2zS)
   * documentation for more information.
   *
   * @return - The number of rows affected as an `int`.
   */
  <<__HipHopSpecific, __Native>>
  function numRowsAffected(): int;

  /**
   * The last ID inserted, if one existed, for the query that produced the
   * current result.
   *
   * See the MySQL's [mysql_insert_id()](http://goo.gl/qxIcPz) documentation for
   * more information.
   *
   * @return - The last insert id, or 0 if none existed.
   */
  <<__HipHopSpecific, __Native>>
  function lastInsertId(): int;

  /**
   * The number of rows in the current result.
   *
   * This is particularly useful for `SELECT` statements.
   *
   * This is complementary to `numRowsAffected()` as they might be the same
   * value, but if this was an `INSERT` query, for example, then this might be
   * 0, while `numRowsAffected()` could be non-zero.
   *
   * See the MySQL's [mysql_num_rows()](http://goo.gl/Rv5NaL) documentation for
   * more information.
   *
   * @return - The number of rows in the current result as an `int`.
   */
  <<__HipHopSpecific, __Native>>
  function numRows(): int;

  /**
   * Returns the actual rows returned by the successful query, each row
   * including the name and value for each column.
   *
   *  All values come back as `string`s. If you want typed values, use
   * `mapRowsTyped()`.
   *
   * The rows are returned as a `Vector` of `Map` objects. The `Map` objects map
   * column names to (possibly `null`) `string` values.
   *
   * @return - A `Vector` of `Map` objects, where the `Vector` elements are the
   *           rows and the `Map` elements are the column names and values
   *           associated with that row.
   */
  <<__HipHopSpecific, __Native>>
  function mapRows(): Vector<Map>;

  /**
   * Returns the actual rows returned by the successful query, each row
   * including the values for each column.
   *
   * All values come back as `string`s. If you want typed values, use
   * `vectorRowsTyped()`.
   *
   * The rows are returned as a `Vector` of `Vector` objects which hold the
   * (possibly `null`) `string` values of each column in the order of the
   * original query.
   *
   * @return - A `Vector` of `Vector` objects, where the outer `Vector`
   *           represents the rows and each inner `Vector` represent the
   *           column values for each row.
   */
  <<__HipHopSpecific, __Native>>
  function vectorRows(): Vector;

  /**
   * Returns the actual rows returned by the successful query, each row
   * including the name and typed-value for each column.
   *
   * The rows are returned as a `Vector` of `Map` objects. The `Map` objects map
   * column names to (possibly `null`) `mixed` values (e.g., an `INTEGER` column
   * will come back as an `int`.)
   *
   * @return - A `Vector` of `Map` objects, where the `Vector` elements are the
   *           rows and the `Map` elements are the column names and typed values
   *           associated with that row.
   */
  <<__HipHopSpecific, __Native>>
  function mapRowsTyped(): Vector;

  /**
   * Returns the actual rows returned by the successful query, each row
   * including the typed values for each column.
   *
   * The rows are returned as a `Vector` of `Vector` objects which hold the
   * (possibly `null`) `mixed` values of each column in the order of the
   * original query (e.g., an `INTEGER` column will come back as an `int`.).
   *
   * @return - A `Vector` of `Vector` objects, where the outer `Vector`
   *           represents the rows and each inner `Vector` represent the typed
   *           column values for each row.
   */
  <<__HipHopSpecific, __Native>>
  function vectorRowsTyped(): Vector;

  /**
   * Returns a `Vector` representing all row blocks returned by the successful
   * query.
   *
   * A row block can be the full result of the query (if there is only one
   * row block), or it can be the partial result of the query (if there are
   * more than one row block). The total number of row blocks makes up the
   * entire result of the successful query.
   *
   * Usually, there will be only one row block in the vector because the
   * query completed in full in one attempt. However, if, for example, the
   * query represented something that exceeded some network parameter, the
   * result could come back in multiple blocks.
   *
   * @return - A `Vector` of `AsyncMysqlRowBlock` objects, the total number
   *           of which represent the full result of the query.
   */
  <<__HipHopSpecific, __Native>>
  function rowBlocks(): Vector<AsyncMysqlRowBlock>;
}

/**
 * Represents a row block.
 *
 * A row block is either a full or partial set of result rows from a MySQL
 * query.
 *
 * In a query result, the sum total of all the row blocks is the full result
 * of the query. Most of the time there is only one row block per query result
 * since the query was never interrupted or otherwise deterred by some outside
 * condition like exceeding network packet parameters.
 *
 * You can get an instance of `AsyncMysqlRowBlock` via the
 * `AsyncMysqlQueryResult::rowBlocks()` call.
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 */
<<__NativeData("AsyncMysqlRowBlock")>>
final class AsyncMysqlRowBlock implements IteratorAggregate, Countable {
  /**
   * @internal
   */
  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Get a field (column) value.
   *
   * @param $row - the row index.
   * @param $field - the field index (`int`) or field name (`string`).
   *
   * @return - The value of the field (column) at the given row.
   *
   */
  <<__HipHopSpecific, __Native>>
  function at(int $row, mixed $field): mixed;

  /**
   * Get a certain field (column) value from a certain row as `int`.
   *
   * If the column from which you are retrieving the value is not an integral
   * type, then an `Exception` is thrown.
   *
   * @param $row - the row index.
   * @param $field - the field index (`int`) or field name (`string`).
   *
   * @return - The `int` value of the field (column); or an `Exception` if it
   *           the column is not integral.
   */
  <<__HipHopSpecific, __Native>>
  function getFieldAsInt(int $row, mixed $field): int;

  /**
   * Get a certain field (column) value from a certain row as `double`.
   *
   * If the column from which you are retrieving the value is not an numeric
   * type, then an `Exception` is thrown.
   *
   * @param $row - the row index.
   * @param $field - the field index (`int`) or field name (`string`).
   *
   * @return - The `double` value of the field (column); or an `Exception` if it
   *           the column is not numeric.
   */
  <<__HipHopSpecific, __Native>>
  function getFieldAsDouble(int $row, mixed $field): float;

  /**
   * Get a certain field (column) value from a certain row as `string`.
   *
   * @param $row - the row index.
   * @param $field - the field index (`int`) or field name (`string`).
   *
   * @return - The `string` value of the field (column).
   */
  <<__HipHopSpecific, __Native>>
  function getFieldAsString(int $row, mixed $field): string;

  /**
   * Returns whether a field (column) value is `null`.
   *
   * @param $row - the row index.
   * @param $field - the field index (`int`) or field name (`string`).
   *
   * @return - `true` if the column value is `null`; `false` otherwise.
   */
  <<__HipHopSpecific, __Native>>
  function isNull(int $row, mixed $field): bool;

  /**
   * Returns the type of the field (column).
   *
   * See [here](http://goo.gl/TbnKJy) and [here](https://goo.gl/aSEMeg) for the
   * integer mappings to SQL types.
   *
   * @param $field - the field index (`int`) or field name (`string`).
   *
   * @return - The type of the field as an `int`.
   */
  <<__HipHopSpecific, __Native>>
  function fieldType(mixed $field): int;

  /**
   * Returns the flags of the field (column).
   *
   * This gets the bitwise `OR` of the flags that are set for a given column.
   *
   * See [here](http://goo.gl/1RCN2l) for the possible flags.
   *
   * @param $field - the field index (`int`) or field name (`string`).
   *
   * @return - The flags of the column as an `int`.
   */
  <<__HipHopSpecific, __Native>>
  function fieldFlags(mixed $field): int;

  /**
   * Returns the name of the field (column).
   *
   * @param $field - the field index.
   *
   * @return - The name of the column as a `string`.
   */
  <<__HipHopSpecific, __Native>>
  function fieldName(int $field): string;

  /**
   * Returns whether there were any rows are returned in the current row block.
   *
   * @return - `true` if there are rows; `false` otherwise.
   */
  <<__HipHopSpecific, __Native>>
  function isEmpty(): bool;

  /**
   * Returns the number of fields (columns) associated with the current row
   * block.
   *
   * @return - The number of columns in the current row block.
   */
  <<__HipHopSpecific, __Native>>
  function fieldsCount(): int;

  /**
   * Returns the number of rows in the current row block.
   *
   * @return - The number of rows in the current row block.
   */
  <<__HipHopSpecific, __Native>>
  function count(): int;

  /**
   * Get the iterator for the rows in the block.
   *
   * @return - An `AsyncMysqlRowBlockIterator` to iterate over the current
   *            row block.
   */
  <<__HipHopSpecific, __Native>>
  function getIterator(): AsyncMysqlRowBlockIterator;

  /**
   * Get a certain row in the current row block.
   *
   * @param $row - the row index.
   *
   * @return - The `AsyncMysqlRow` representing one specific row in the current
   *           row block.
   */
  <<__HipHopSpecific, __Native>>
  function getRow(int $row): AsyncMysqlRow;
}

/**
 * A class to represent an iterator over the rows of a `AsyncMysqlRowBlock`.
 *
 * You can iterate over all the rows of an `AsyncMysqlRowBlock` one by one until
 * the iterator is not valid any longer.
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 */
<<__NativeData("AsyncMysqlRowBlockIterator")>>
final class AsyncMysqlRowBlockIterator implements HH\KeyedIterator {

  /**
   * @internal
   */
  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Check if iterator is at a valid `AsyncMysqlRow`.
   *
   * @return - `true` if the iterator is still pointing to a valid row;
   *            otherwise `false`.
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
   * @return - The `AsyncMysqlRow` associated with the current iterator
   *           position.
   */
  <<__HipHopSpecific, __Native>>
  function current(): AsyncMysqlRow;

  /**
   * Get the current row number
   *
   * @return - The current row number associated with the current iterator
   *           position.
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
 * A class to represent a row.
 *
 * You can think of a row just like you do a database row that might be
 * returned as a result from a query. The row has values associated with
 * each column.
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 */
<<__NativeData("AsyncMysqlRow")>>
final class AsyncMysqlRow implements MysqlRow {

  /**
   * @internal
   */
  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Get field (column) value indexed by the `field`.
   *
   * @param $field - the field index (`int`) or field name (`string`).
   *
   * @return - The value of the field (column).
   *
   */
  <<__HipHopSpecific, __Native>>
  function at(mixed $field): mixed;

  /**
   * Get a certain field (column) value as an `int`.
   *
   * If the column from which you are retrieving the value is not an integral
   * type, then an `Exception` is thrown.
   *
   * @param $field - the field index (`int`) or field name (`string`).
   *
   * @return - The `int` value of the field (column); or an `Exception` if it
   *           the column is not integral.
   */
  <<__HipHopSpecific, __Native>>
  function getFieldAsInt(mixed $field): int;

  /**
   * Get a certain field (column) value as a `double`.
   *
   * If the column from which you are retrieving the value is not an numeric
   * type, then an `Exception` is thrown.
   *
   * @param $field - the field index (`int`) or field name (`string`).
   *
   * @return - The `double` value of the field (column); or an `Exception` if it
   *           the column is not numeric.
   */
  <<__HipHopSpecific, __Native>>
  function getFieldAsDouble(mixed $field): float;

  /**
   * Get a certain field (column) value as a `string`.
   *
   * @param $field - the field index (`int`) or field name (`string`).
   *
   * @return - The `string` value of the field (column).
   */
  <<__HipHopSpecific, __Native>>
  function getFieldAsString(mixed $field): string;

  /**
   * Returns whether a field (column) value is `null`.
   *
   * @param $field - the field index (`int`) or field name (`string`).
   *
   * @return - `true` if the column value is `null`; `false` otherwise.
   */
  <<__HipHopSpecific, __Native>>
  function isNull(mixed $field): bool;

  /**
   * Returns the type of the field (column).
   *
   * See [here](http://goo.gl/TbnKJy) and [here](https://goo.gl/aSEMeg) for the
   * integer mappings to SQL types.
   *
   * @param $field - the field index (`int`) or field name (`string`).
   *
   * @return - The type of the field as an `int`.
   */
  <<__HipHopSpecific, __Native>>
  function fieldType(mixed $field): int;

  /**
   * Get the number of fields (columns) in the current row.
   *
   * @return - The number of columns in the current row.
   */
  <<__HipHopSpecific, __Native>>
  function count(): int;

  /**
   * Get the iterator over the fields in the current row.
   *
   * @return - An `AsyncMysqlRowIterator` to iterate over the current row.
   */
  <<__HipHopSpecific, __Native>>
  function getIterator(): AsyncMysqlRowIterator;
}

/**
 * A class to represent an iterator over the fields (columns) in a row.
 *
 * You can iterate over all the fields (columns) of an `AsyncMysqlBlock` one by
 * one until the iterator is not valid any longer.
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/extensions
 */
<<__NativeData("AsyncMysqlRowIterator")>>
final class AsyncMysqlRowIterator implements HH\KeyedIterator {
  /**
   * @internal
   */
  private function __construct(): void {
    throw new InvalidOperationException(
      __CLASS__ . " objects cannot be directly created");
  }

  /**
   * Check if the iterator is at a valid field (column).
   *
   * @return - `true` if the iterator is still pointing to a valid column;
   *            otherwise `false`.
   */
  <<__HipHopSpecific, __Native>>
  function valid(): bool;

  /**
   * Advance the iterator to the next field (column).
   *
   */
  <<__HipHopSpecific, __Native>>
  function next(): void;

  /**
   * Get the current field (column) name.
   *
   * @return - The column name associated with the current iterator
   *           position.
   */
  <<__HipHopSpecific, __Native>>
  function current(): string;

  /**
   * Get the current field (column) number.
   *
   * @return - The column number associated with the current iterator position.
   *
   */
  <<__HipHopSpecific, __Native>>
  function key(): int;

  /**
   * Reset the iterator to the first field (column).
   *
   */
  <<__HipHopSpecific, __Native>>
  function rewind(): void;
}
