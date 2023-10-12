<?hh // strict

namespace HH\Lib\Dict\Dict {

  function f(): string {
    return 'foo';
  }

}

namespace HH\Lib\Dict {

  function f(): int {
    return 1;
  }

  function main(): void {
    expect_int(Dict\f()); // ok
  }

  function expect_int(int $x): void {}

}
