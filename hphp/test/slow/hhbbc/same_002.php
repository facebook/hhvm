<?hh

function foo($x) {
  $a = $x ? "a" : null;
  $b = "b";

  if ($a !== $b) return $a;
  return $a . $b;
}


<<__EntryPoint>>
function main_same_002() {
var_dump(foo(true));
var_dump(foo(false));
}
