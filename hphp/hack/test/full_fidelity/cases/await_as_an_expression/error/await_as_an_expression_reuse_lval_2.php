<?hh

async function foo(): Awaitable<void> {
  $x = (await genx()) + ($y = 42) + $y;
}
