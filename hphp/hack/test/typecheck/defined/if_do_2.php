<?hh

function foo(bool $b): int {
  if ($b) {
    do {
      $x = 1;
      $y = 42;
    } while (0);
  } else {
    do {
      $x = 2;
    } while (0);
  }
  return $y;
}
