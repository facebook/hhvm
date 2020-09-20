<?hh

namespace foo {
  function bar() {
    return __FUNCTION__;
  }

  function baz() {
    return __FUNCTION__;
  }
}

namespace fizz {
  function baz() {
    return __FUNCTION__;
  }
}

namespace baz {
  function bar() {
    return __FUNCTION__;
  }

  function baz() {
    return __FUNCTION__;
  }
}

namespace test_as {
  use function baz\bar;
  use function fizz\baz;
  use function foo\bar as foobar;
  use function fizz\baz as fizzbaz;
  use function baz\baz as bazbaz;

  <<__EntryPoint>> function main(): void {
    \var_dump(bar());
    \var_dump(baz());
    \var_dump(foobar());
    \var_dump(fizzbaz());
    \var_dump(bazbaz());
  }
}
