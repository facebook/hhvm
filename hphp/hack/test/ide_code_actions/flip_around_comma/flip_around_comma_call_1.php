<?hh

function foo(): void {
  bar(1, /*range-start*//*range-end*/2);
}

function bar(int $a, int $b): void {}
