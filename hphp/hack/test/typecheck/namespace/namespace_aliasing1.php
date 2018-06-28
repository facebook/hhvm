<?hh

namespace HH\Lib\Dict {
  function foo(): int {
    return 1;
  }
}
namespace {
  function main() {
    expect_int(Dict\foo()); // ok
    expect_int(\Dict\foo()); // ok TODO(T22617428) should be error
    expect_int(HH\Lib\Dict\foo()); // ok
    expect_int(\HH\Lib\Dict\foo()); // ok
  }

  function expect_int(int $x): void {}
}
