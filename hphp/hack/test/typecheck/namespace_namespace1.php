<?hh // strict

namespace Foo {
  function f(): int {
    $x = namespace\Bar\g();
    hh_show($x);
    return $x;
  }
}

namespace Foo\Bar {
  function g(): int {
    return 0;
  }
}
