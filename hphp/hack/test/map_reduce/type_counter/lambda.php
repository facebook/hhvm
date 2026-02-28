<?hh

function f(int $x): int {
  $f = (int $y) ==> $x+$y;
  return $f(3);
}
