<?php
namespace __SystemLib {
class Error implements Throwable {
  use BaseException;
  /**
   * ( excerpt from http://php.net/manual/en/exception.construct.php )
   *
   * Constructs the Exception.
   *
   * @message    mixed   The Exception message to throw.
   * @code       mixed   The Exception code.
   * @previous   mixed   The previous exception used for the exception
   *                     chaining.
   */
  public function __construct($message = '', $code = 0,
                              \__SystemLib\Throwable $previous = null) {

    // Child classes may just override the protected property
    // without implementing a constructor or calling parent one.
    // In this case we should not override it from the param.

    if ($message !== '' || $this->message === '') {
      $this->message = $message;
    }

    if ($code !== 0 || $this->code === 0) {
      $this->code = $code;
    }

    $this->previous = $previous;
  }
}

class ArithmeticError extends Error {}
class AssertionError extends Error {}
class DivisionByZeroError extends Error {}
class ParseError extends Error {}
class TypeError extends Error {}
}
