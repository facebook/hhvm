<?hh // strict

function f(inout int $i): void {}

function launder(): bool {
  return false;
}

function test(): void {
  if (launder()) {
    $x = Pair { 5, 42 };
  } else {
    $x = varray[1, 2, 3];
  }
  f(inout $x[1]);
}
