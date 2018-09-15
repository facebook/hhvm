<?hh // strict

<<__Rx>>
async function f(): Awaitable<int> {
  return 1;
}

<<__Rx>>
async function g(): Awaitable<void> {
  // error
  $a = f();
  $b = await $a;
}
