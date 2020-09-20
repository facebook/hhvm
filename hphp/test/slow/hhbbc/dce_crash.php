<?hh

function f($x) { return $x; }

function g($t) {
  $a = 1;
  $b = 1;
  return f($t ? $a : $b);
}


<<__EntryPoint>>
function main_dce_crash() {
var_dump(g(true));
}
