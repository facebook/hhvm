<?hh

function launder(): bool {
  return false;
}

function f(inout string $s): void {
  if (launder()) {
    $s = 3.14159;
  } else {
    $s = 'fizzbuzz';
  }
}
