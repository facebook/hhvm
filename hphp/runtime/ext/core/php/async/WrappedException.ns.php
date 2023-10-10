<?hh

namespace HH\Asio {

/**
 * Represents the result of failed `Awaitable` operation.
 */
final class WrappedException<Te as \Exception,Tr>
  implements ResultOrExceptionWrapper<Tr> {
  /**
   * Instantiate a `WrappedException`.
   *
   * Normally, you will not instantiate a `WrappedException` directly, but
   * rather have one returned back to you on a call to `wrap()` if the
   * operation failed.
   *
   * @param  $result - The exception thrown during the `Awaitable` operation.
   */
  public function __construct(private Te $exception) {}

  /**
   * Since this is a failed result wrapper, this always returns `false`.
   *
   * @return - `false`.
   */
  public function isSucceeded(): bool {
    return false;
  }

  /**
   * Since this is a failed result wrapper, this always returns `true`.
   *
   * @return - `true`.
   */
  public function isFailed(): bool {
    return true;
  }

  /**
   * Since this is a failed result wrapper, this always returns the exception
   * thrown during the `Awaitable` operation.
   *
   * `getResult()` is the same as `getException() in this case.
   *
   * @return - The exception thrown during the `Awaitable` operation.
   */
  public function getResult(): Tr {
    throw $this->exception;
  }

  /**
   * Since this is a failed result wrapper, this always returns the exception
   * thrown during the `Awaitable` operation.
   *
   * `getException()` is the same as `getResult() in this case.
   *
   * @return - The exception thrown during the `Awaitable` operation.
   */
  public function getException(): Te {
    return $this->exception;
  }
}

} // namespace HH\Asio
