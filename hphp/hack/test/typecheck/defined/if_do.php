<?hh // strict

function foo(bool $b): int {
  if ($b) {
    do {
      $x = 1;
    } while (0);
  } else {
    do {
      $x = 2;
    } while (0);
  }
  return $x;
}
