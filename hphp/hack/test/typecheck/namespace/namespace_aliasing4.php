<?hh

namespace Dict {
  function foo(): string {
    return 'foo';
  }
}

namespace HH\Lib\Dict {
  function foo(): int {
    return 1;
  }
}

namespace {
  function main(): void {
    expect_int(Dict\foo()); // ok
    expect_int(\Dict\foo()); // error
  }

  function expect_int(int $x): void {}
}
