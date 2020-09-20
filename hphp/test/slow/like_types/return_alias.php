<?hh

function f(mixed $x): MyAlias {
  return $x;
}

function g(mixed $x): void {
  var_dump(f($x));
}

<<__EntryPoint>>
function main(): void {
  include_once 'parameter_alias.inc';

  g(1);
  g(1.5);
  g('foo');
  g(false);
  g(fopen(__FILE__, 'r'));
  g(new stdClass());
  g(tuple(1, 2, 3));
  g(shape('a' => 1, 'b' => 2));
}
