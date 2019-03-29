<?hh

function two() { return 2; }
function bar(bool $x) { return $x ? array('y' => two()) : array(); }
function foo(bool $x) { return array('x' => bar($x)); }
function main(bool $x) {
  $ar = foo($x);
  $ar['x']['y']->x = 42;
  $k = $ar['x']['y']->x;
  var_dump($k, $ar);
}

<<__EntryPoint>>
function main_array_019() {
main(true);
main(false);
}
