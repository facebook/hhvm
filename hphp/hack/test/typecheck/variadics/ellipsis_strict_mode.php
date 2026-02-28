<?hh

function bar((function(): void) $fn): void {}

function test1(): (function(): int) {
  return (...$_) ==> 4;
}

function test2(): (function(): int) {
  $x = (...$_) ==> {};
  $y = (function(...$_) {});

  bar($x);
  bar($y);

  return (
    function(...$_): int {
      return 4;
    }
  );
}

function test3(): void {
  bar((...$_) ==> {});
}

function test4(): void {
  bar(function(...$_): void {});
}

function test5(...$_): void {}
