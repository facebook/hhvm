<?hh

function two() { return 2; }
function bar(bool $x) { return $x ? darray['y' => two()] : darray['y' => new stdClass()]; }
function foo(bool $x) { return darray['x' => bar($x)]; }
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
