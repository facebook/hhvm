<?hh

function f($x) {
  return $x{123}; // error: deprecated subscript syntax
}

function g($a, $b) {
  return $a ? : $b; // error: expected elvis operator
}
