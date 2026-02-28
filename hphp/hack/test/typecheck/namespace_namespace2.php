<?hh

namespace Foo {
  function f(): string {
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
