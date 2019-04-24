<?hh

include_once 'parameter_alias.inc';

/**
 * Like-types are treated as mixed, allowing any argument value.
 */
function f(MyAlias $x): void {
  var_dump($x);
}

<<__EntryPoint>>
function main(): void {
  f(1);
  f(1.5);
  f('foo');
  f(false);
  f(STDIN);
  f(new stdClass());
  f(tuple(1, 2, 3));
  f(shape('a' => 1, 'b' => 2));
}
