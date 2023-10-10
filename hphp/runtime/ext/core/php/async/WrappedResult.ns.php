<?hh

namespace HH\Asio {

/**
 * Represents the result of successful `Awaitable` operation.
 */
final class WrappedResult<T> implements ResultOrExceptionWrapper<T> {
  /**
   * Instantiate a `WrappedResult`.
   *
   * Normally, you will not instantiate a `WrappedResult` directly, but rather
   * have one returned back to you on a call to `wrap()` if the operation
   * succeeded.
   *
   * @param  $result - The result of the `Awaitable` operation.
   */
  public function __construct(private T $result) {}

  /**
   * Since this is a successful result wrapper, this always returns `true`.
   *
   * @return - `true`.
   */
  public function isSucceeded(): bool {
    return true;
  }

  /**
   * Since this is a successful result wrapper, this always returns `false`.
   *
   * @return - `false`.
   */
  public function isFailed(): bool {
    return false;
  }

  /**
   * Since this is a successful result wrapper, this always returns the actual
   * result of the `Awaitable` operation.
   *
   * @return - The result of the `Awaitable` operation.
   */
  public function getResult(): T {
    return $this->result;
  }

  /**
   * Since this is a successful result wrapper, this always returns an
   * `InvariantException` saying that there was no exception thrown from
   * the `Awaitable` operation.
   *
   * @return - An `InvariantException`.
   */
  public function getException(): \Exception {
    invariant_violation('Unable to get exception from WrappedResult');
  }
}

} // namespace HH\Asio
