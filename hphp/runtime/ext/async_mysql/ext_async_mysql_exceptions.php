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
 * The base exception class for any issues arising from the use of async
 * MySQL.
 *
 * In general, when trying to connect to a MySQL client (e.g., via
 * `AsyncMysqlConnectionPool::connect()`) or when making a query (e.g., via
 * `AsyncMysqlConnection::queryf()`), it is good practice to have your code
 * exception catchable somewhere in the code pipeline (via
 * [try/catch](http://php.net/manual/en/language.exceptions.php)).
 *
 * e.g.,
 *
 * ```
 * try {
 *   // code here
 * } catch (AsyncMysqlException $ex) {
 *   $error = $ex->mysqlErrorString();
 * }
 * ```
 *
 * @guide /hack/async/intro/
 * @guide /hack/async/extensions/
 */
class AsyncMysqlException extends Exception {
  private AsyncMysqlErrorResult $result;
  /**
   * Explicitly construct an `AsyncMysqlException`.
   *
   * Normally, you will `catch` an `AsyncMysqlException`, but if you want to
   * explictly construct one and, for example, `throw` it for some given reason,
   * then you pass it an `AsyncMysqlErrorResult`.
   *
   * @param $result - An `AsyncMysqlErrorResult` that contains the error
   *                  information for the failed operation.
   */
  public function __construct(AsyncMysqlErrorResult $result) {
    parent::__construct();

    $this->result = $result;

    $this->message =
      'Error executing AsyncMysql operation: '.$result->failureType();

    if  ($this->mysqlErrorCode() > 0) {
      $this->message = $this->message.' (mysql error: '.$this->mysqlErrorCode().
        ': '.$this->mysqlErrorString().')';
    }
  }

  /**
   * Returns the MySQL error number for that caused the current exception.
   *
   * See MySQL's
   * [mysql_errno()](http://dev.mysql.com/doc/refman/5.0/en/mysql-errno.html)
   * for information on the error numbers.
   *
   * @return - The error number as an `int`.
   */
  public function mysqlErrorCode(): int {
    return $this->result->mysql_errno();
  }

  /**
   * Returns a human-readable string for the error encountered in the current
   * exception.
   *
   * @return - The error string.
   */
  public function mysqlErrorString(): string {
    return $this->result->mysql_error();
  }

  /**
   * Returns whether the type of failure that produced the exception was a
   * timeout.
   *
   * An `AsyncMysqlErrorResult` can occur due to `'TimedOut'`, representing a
   * timeout, or `'Failed'`, representing the server rejecting the connection
   * or query.
   *
   * @return - `true` if the type of failure was a time out; `false` otherwise.
   */
  public function timedOut(): bool {
    return $this->result->failureType() === "TimedOut";
  }

  /**
   * Returns whether the type of failure that produced the exception was a
   * general connection or query failure.
   *
   * An `AsyncMysqlErrorResult` can occur due to `'TimedOut'`, representing a
   * timeout, or `'Failed'`, representing the server rejecting the connection
   * or query.
   *
   * @return - `true` if the type of failure was a general failure; `false`
   *            otherwise.
   */
  public function failed(): bool {
    return $this->result->failureType() === "Failed";
  }

  /**
   * Returns the underlying `AsyncMysqlErrorResult` associated with the current
   * exception.
   *
   * @return - The `AsyncMysqlErrorResult` that underlies the current exception.
   */
  public function getResult(): AsyncMysqlErrorResult {
    return $this->result;
  }
}

/**
 * The exception associated with a MySQL connection failure.
 *
 * All methods are the same as the base `AsyncMysqlException`.
 */
class AsyncMysqlConnectException extends AsyncMysqlException { }

/**
 * The exception associated with a MySQL query failure.
 *
 * All methods are the same as the base `AsyncMysqlException`.
 */
class AsyncMysqlQueryException extends AsyncMysqlException { }
