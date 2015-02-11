<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

namespace HH\Asio {

/**
 * Represents a result of operation that may have failed.
 */
interface ResultOrExceptionWrapper<T> {
  /**
   * Return true iff the operation succeeded.
   */
  public function isSucceeded(): bool;

  /**
   * Return true iff the operation failed.
   */
  public function isFailed(): bool;

  /**
   * Return the result of the operation, or throw underlying exception.
   *
   * - if the operation succeeded: return its result
   * - if the operation failed: throw the exception incating failure
   */
  public function getResult(): T;

  /**
   * Return the underlying exception, or fail with invariant violation.
   *
   * - if the operation succeeded: fails with invariant violation
   * - if the operation failed: returns the exception indicating failure
   */
  public function getException(): Exception;
}

} // namespace HH\Asio
