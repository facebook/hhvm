<?hh

namespace foo {
  class MyException extends \Exception {}
}

namespace {
  use type foo\MyException as InvariantException;
  \var_dump(InvariantException::class);
}
