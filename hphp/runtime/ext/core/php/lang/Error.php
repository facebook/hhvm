<?hh

namespace {

class Error implements Throwable {
  use \__SystemLib\BaseException;
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
  public function __construct(
    mixed $message = '',
    mixed $code = 0,
    ?Throwable $previous = null,
  )[] {

    // Child classes may just override the protected property
    // without implementing a constructor or calling parent one.
    // In this case we should not override it from the param.

    if ($message !== '' || $this->message === '') {
      $this->message = HH\FIXME\UNSAFE_CAST<mixed, string>($message);
    }

    if ($code !== 0 || $this->code === 0) {
      $this->code = HH\FIXME\UNSAFE_CAST<mixed, int>($code);
    }

    $this->previous = $previous;
  }
}

class ArithmeticError extends Error {}
class ArgumentCountError extends Error {}
class AssertionError extends Error {}
class DivisionByZeroError extends Error {}
class ParseError extends Error {}
class TypeError extends Error {}

}
