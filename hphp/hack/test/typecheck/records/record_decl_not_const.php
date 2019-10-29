<?hh

function foo(): int { return 1; }

record A {
  x: int = foo(),
}
