<?hh

function foo(): int { return 1; }

record A {
  int x = foo();
}
