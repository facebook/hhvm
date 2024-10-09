<?hh
<<file:__EnableUnstableFeatures('like_type_hints')>>

/**
 * Like-types are enforced at their inner type.
 */
function f(<<__Soft>> MyAlias $x): void {
  var_dump($x);
}

<<__EntryPoint>>
function main(): void {
  include_once 'parameter_alias.inc';

  f(1);
  f(1.5);
  f('foo');
  f(false);
  f(fopen(__FILE__, 'r'));
  f(new stdClass());
  f(tuple(1, 2, 3));
  f(shape('a' => 1, 'b' => 2));
}
