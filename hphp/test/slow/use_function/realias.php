<?hh

namespace foo {
  function bar() :mixed{
    return __FUNCTION__;
  }
}

namespace {
  use function foo\bar as foobar;
  use function foobar as foo_bar;
  <<__EntryPoint>> function main(): void {
    \var_dump(foobar());
    \var_dump(foo_bar());
  }
}
