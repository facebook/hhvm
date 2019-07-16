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

function test5(...): void {}
