<?hh // strict

async function f()[rx]: Awaitable<int> {
  return 1;
}


async function g()[rx]: Awaitable<void> {
  // error
  $a = f();
  $b = await $a;
}
