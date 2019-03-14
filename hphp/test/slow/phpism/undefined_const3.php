<?hh

namespace {
  const SOME_CONST = 1;
}

namespace Foo {
  use const Bar\SOME_CONST;

  <<__EntryPoint>>
  function f(): void {
    \var_dump(SOME_CONST);
  }
}
