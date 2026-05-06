<?hh

function foo(bool $b): int {
  if ($b) {
    do {
      $x = 1;
      $y = 42;
    } while (false);
  } else {
    do {
      $x = 2;
    } while (false);
  }
  return $y;
}
