<?hh

async function genx(): Awaitable<int> { return 42; }

function foo(): Awaitable<void> {
  $x = await genx() + 42;
  return $x;
}
