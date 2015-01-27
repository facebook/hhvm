<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

namespace HH\Asio {

/**
 * Represents a result of succeeded operation.
 */
final class WrappedResult<T> implements ResultOrExceptionWrapper<T> {
  public function __construct(private T $result) {}

  public function isSucceeded(): bool {
    return true;
  }

  public function isFailed(): bool {
    return false;
  }

  public function getResult(): T {
    return $this->result;
  }

  public function getException(): Exception {
    invariant_violation('Unable to get exception from WrappedResult');
  }
}

} // namespace HH\Asio
