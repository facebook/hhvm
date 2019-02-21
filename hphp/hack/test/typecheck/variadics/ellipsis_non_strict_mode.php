<?hh // partial

function bar((function(): void) $fn): void {}

function test1(): (function(): int) {
  return (...) ==> 4;
}

function test2(): (function(): int) {
  $x = (...) ==> {};
  $y = (function(...) {});

  bar($x);
  bar($y);

  return (
    function(...): int {
      return 4;
    }
  );
}

function test3(): void {
  bar((...) ==> {});
}

function test4(): void {
  bar(function(...): void {});
}

function test5(): (function(...): int) {
  return (...$args) ==> 4;
}

class A {
  public function test6(): (function(...): int) {
    return (...$args) ==> 4;
  }
}

function test7((function(...): void) $fn): void {}

function test8(...): void {}
