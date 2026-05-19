<?hh

<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>;

async function gen_shape(): Awaitable<shape('x' => int)> {
  return shape('x' => 20);
}

async function gen_tuple(): Awaitable<(int, string)> {
  return tuple(30, 'thirty');
}

function test_shape_literal_rhs(): void {
  echo "test_shape_literal_rhs\n";
  shape('x' => $x) = shape('x' => 10);
  echo "x="; var_dump($x);
}

async function test_shape_await(): Awaitable<void> {
  echo "test_shape_await\n";
  shape('x' => $x) = await gen_shape();
  echo "x="; var_dump($x);
}

async function test_tuple_await(): Awaitable<void> {
  echo "test_tuple_await\n";
  tuple($n, $label) = await gen_tuple();
  echo "n="; var_dump($n);
  echo "label="; var_dump($label);
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  test_shape_literal_rhs();
  await test_shape_await();
  await test_tuple_await();
}
