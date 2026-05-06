<?hh

function foo(bool $b): int {
  if ($b) {
    do {
      $x = 1;
    } while (false);
  } else {
    do {
      $x = 2;
    } while (false);
  }
  return $x;
}
