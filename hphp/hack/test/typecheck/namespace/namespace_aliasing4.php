<?hh

namespace Dict {
  function foo(): string {
    return 'foo';
  }
}

namespace HH\Lib\Dict {
  function foo(): int { // error
    return 1;
  }
}
namespace {
  function main() {
    expect_int(Dict\foo()); // error
    expect_int(\Dict\foo()); // error
  }

  function expect_int(int $x): void {}
}
