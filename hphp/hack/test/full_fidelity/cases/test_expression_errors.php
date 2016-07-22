<?hh

function f($x) : void {
  return $x{123}; // error; deprecated subscript syntax
}
