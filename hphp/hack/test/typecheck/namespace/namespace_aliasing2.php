<?hh // partial

namespace HH\SomethingElse\Dict {
  function foo(): int {
    return 1;
  }
}

namespace {
  function main() {
    expect_int(Dict\foo()); // error
    expect_int(\Dict\foo()); // error
    expect_int(HH\SomethingElse\Dict\foo()); // ok
    expect_int(\HH\SomethingElse\Dict\foo()); // ok
  }

  function expect_int(int $x): void {}
}
