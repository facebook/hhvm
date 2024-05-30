<?hh

function foo(): int {
  return 1;
}

function bar(): bool {
  return foo();
}
