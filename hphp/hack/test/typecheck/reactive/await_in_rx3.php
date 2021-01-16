<?hh // strict
<<__Rx>>
async function f()[rx]: Awaitable<int> {
  return 1;
}

<<__Rx>>
async function g()[rx]: Awaitable<void> {
  // OK
  $a = await f();
}
