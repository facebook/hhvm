<?hh

function f(mixed $x): int {
  return 2;
}
function x(int $x): int {
  $z = vec[];
  $ret = $x + f($x = $z);
  return $ret;
}
