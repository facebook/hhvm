<?hh // strict

<<__Rx>>
async function genx(): Awaitable<int> { return 42; }
<<__Rx>>
async function geny(): Awaitable<void> {}

<<__Rx>>
async function foo(): Awaitable<int> {
  concurrent {
    $x = await genx();
    await geny();
    $y = await genx();
  }
  return $x + $y;
}
