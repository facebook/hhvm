<?hh

namespace HH\Asio {

/**
 * Represents a result of operation that either has a successful result of an
 * `Awaitable` or the exception object if that `Awaitable` failed.
 *
 * This is an interface. You get generally `ResultOrExceptionWrapper` by calling
 * `wrap()`, passing in the `Awaitable`, and a `WrappedResult` or
 * `WrappedException` is returned.
 */
interface ResultOrExceptionWrapper<T> {
  /**
   * Indicates whether the `Awaitable` associated with this wrapper exited
   * normally.
   *
   * If `isSucceeded()` returns `true`, `isFailed()` returns `false`.
   *
   * @return - `true` if the `Awaitable` succeeded; `false` otherwise.
   */
  public function isSucceeded(): bool;

  /**
   * Indicates whether the `Awaitable` associated with this wrapper exited
   * abnormally via an exception of somoe sort.
   *
   * If `isFailed()` returns `true`, `isSucceeded()` returns `false`.
   *
   * @return - `true` if the `Awaitable` failed; `false` otherwise.
   */
  public function isFailed(): bool;

  /**
   * Return the result of the operation, or throw underlying exception.
   *
   * - if the operation succeeded: return its result
   * - if the operation failed: throw the exception incating failure
   *
   * @return - the result of the `Awaitable` operation upon success or the
   *           exception that was thrown upon failed.
   */
  public function getResult(): T;

  /**
   * Return the underlying exception, or fail with invariant violation.
   *
   * - if the operation succeeded: fails with invariant violation
   * - if the operation failed: returns the exception indicating failure
   *
   * @return - The exception that the `Awaitable` threw upon failure, or an
   *           `InvariantException` if there is no exception (i.e., because
   *           the `Awaitable` succeeded).
   */
  public function getException(): \Exception;
}

} // namespace HH\Asio
