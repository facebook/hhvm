<?hh // strict

<<__Rx>>
async function genx(): Awaitable<int> { return 42; }
<<__Rx>>
async function geny(): Awaitable<int> { return 43; }

<<__Rx>>
async function foo(): Awaitable<int> {
  return (await genx()) + (await geny());
}
