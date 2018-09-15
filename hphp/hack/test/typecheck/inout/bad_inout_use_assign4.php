<?hh // strict

function launder(): bool {
  return false;
}

function f(inout string $s): void {
  if (launder()) {
    g(inout $s);
  } else {
    $s = 'fizzbuzz';
  }
}

function g(inout arraykey $a): void {}
