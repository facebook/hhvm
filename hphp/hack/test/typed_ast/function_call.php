<?hh

function foo(int $x): void {
}

function bar(): void {
  foo(1 + 2);
}
