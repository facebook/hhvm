<?hh
function g($x) {
  return $x;
}

function f($y) {
  return g(1 + $y) == g(1 + $y);
}
