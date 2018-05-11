<?hh // strict

async function foo(Awaitable<int> $i): Awaitable<void> {
  let val : int = await $i;
}
