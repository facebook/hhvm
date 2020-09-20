<?hh

function f($x, $y) {
  $z = 32;
  return $x && $y ?: $z;
}

<<__EntryPoint>>
function main_1847() {
var_dump(f(false, false));
var_dump(f(true, true));
}
