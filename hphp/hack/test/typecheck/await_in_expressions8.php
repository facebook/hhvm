<?hh // strict

async function f1(Awaitable<string> $a): Awaitable<void> {
  throw (await $a);
}
