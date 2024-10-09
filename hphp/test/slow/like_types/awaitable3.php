<?hh
<<file:__EnableUnstableFeatures('like_type_hints')>>

/**
 * For non-async functions, a like-Awaitable acts like any other like type, enforced at the inner type.
 */
function f(mixed $x): <<__Soft>> ~Awaitable<int> {
  return $x;
}

function g(mixed $x): void {
  var_dump(f($x));
}

<<__EntryPoint>>
function main(): void {
  g(1);
  g(1.5);
  g('foo');
  g(false);
  g(fopen(__FILE__, 'r'));
  g(new stdClass());
  g(tuple(1, 2, 3));
  g(shape('a' => 1, 'b' => 2));
}
