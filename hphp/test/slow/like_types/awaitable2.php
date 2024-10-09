<?hh
<<file:__EnableUnstableFeatures('like_type_hints')>>

/**
 * Like-types are enforced at their inner type, so this enforces as Awaitable<int>
 */
async function f(mixed $x): Awaitable<<<__Soft>> ~int> {
  return $x;
}

async function g(mixed $x): Awaitable<void> {
  $y = await f($x);
  var_dump($y);
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  await g(1);
  await g(1.5);
  await g('foo');
  await g(false);
  await g(fopen(__FILE__, 'r'));
  await g(new stdClass());
  await g(tuple(1, 2, 3));
  await g(shape('a' => 1, 'b' => 2));
}
