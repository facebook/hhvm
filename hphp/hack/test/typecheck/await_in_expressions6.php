<?hh // strict

async function f1(Awaitable<int> $a): Awaitable<void> {
  do {
  } while (await $a);
}
