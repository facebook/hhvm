<?hh // strict

async function f()[rx]: Awaitable<int> {
  return 1;
}

async function g(): Awaitable<void> {
  $l = varray[f(), f()];
  $a = await gena($l); // OK
}
