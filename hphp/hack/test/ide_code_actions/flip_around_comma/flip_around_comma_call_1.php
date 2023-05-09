<?hh

function foo(): void {
  bar(1, 2);
}

function bar(int $a, /*range-start*//*range-end*/int $b): void {}
