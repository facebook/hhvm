<?hh // partial

namespace HH\Lib\Dict {
  function foo(): string {
    return 'foo';
  }
}

namespace Some\Other\Dict {
  function foo(): int {
    return 1;
  }
}

namespace {
  use namespace Some\Other\Dict;

  function main() {
    expect_int(Dict\foo()); // ok
  }

  function expect_int(int $x): void {}
}
