<?hh

function two() :mixed{ return 2; }
function bar(bool $x) :mixed{ return $x ? dict['y' => two()] : dict['y' => new stdClass()]; }
function foo(bool $x) :mixed{ return dict['x' => bar($x)]; }
function main(bool $x) :mixed{
  $ar = foo($x);
  $ar['x']['y']->x = 42;
  $k = $ar['x']['y']->x;
  var_dump($k, $ar);
}

<<__EntryPoint>>
function main_array_019() :mixed{
main(true);
main(false);
}
