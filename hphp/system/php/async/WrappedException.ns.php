<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

namespace HH\Asio {

/**
 * Represents a result of failed operation.
 */
final class WrappedException<Te as Exception,Tr>
  implements ResultOrExceptionWrapper<Tr> {
  public function __construct(private Te $exception) {}

  public function isSucceeded(): bool {
    return false;
  }

  public function isFailed(): bool {
    return true;
  }

  public function getResult(): Tr {
    throw $this->exception;
  }

  public function getException(): Te {
    return $this->exception;
  }
}

} // namespace HH\Asio
