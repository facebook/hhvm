<?hh // strict

async function f()[rx]: Awaitable<int> {
  return 1;
}


async function g()[rx]: Awaitable<void> {
  // OK
  $a = await f();
}
