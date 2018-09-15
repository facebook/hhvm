<?hh // strict

<<__Rx>>
async function f(): Awaitable<int> {
  return 1;
}

async function g(): Awaitable<void> {
  // OK
  $a = genva(f(), f());
}
