<?hh // strict

<<__Rx>>
async function f(): Awaitable<int> {
  return 1;
}

<<__Rx>>
async function g(): Awaitable<void> {
  // ERROR
  $l = array(f(), f());
  $a = await gena($l);
}
