<?hh

function foo($p) {
  return $p;
}

$a = 42;
var_dump(foo(true ? 0 : 1));
foo($a);
