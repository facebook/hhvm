<?hh

function f(bool $b): int {
  do {
    if ($b) {
      break;
    }
    $x = 1;
  } while ($x == 0); // no error
  return $x; // undefined error
}
