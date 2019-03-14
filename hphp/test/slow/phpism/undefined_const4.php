<?hh

namespace {
  const SOME_CONST = 1;
}

namespace Foo {
  use const Bar\SOME_CONST;
  \var_dump(SOME_CONST);
}
