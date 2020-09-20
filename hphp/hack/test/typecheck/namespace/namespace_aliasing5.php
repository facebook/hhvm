<?hh // partial

namespace Dict\Foo\Bar {
  function foo(): int {
    return 1;
  }
}

namespace HH\Lib\Dict\Foo\Bar {
  function foo(): string {
    return 'foo';
  }
}

namespace {
  use namespace Dict\Foo\Bar;

  function main() {
    expect_int(Bar\foo()); // ok
  }

  function expect_int(int $x): void {}
}
