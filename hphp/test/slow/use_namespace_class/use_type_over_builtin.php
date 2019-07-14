<?hh

namespace foo {
  class MyException extends \Exception {}
}

namespace {
  use type foo\MyException as InvariantException;
  <<__EntryPoint>> function main(): void {
  \var_dump(InvariantException::class);
  }
}
