<?hh // strict

async function f1(Awaitable<int> $a): Awaitable<void> {
  while ((await $a) > 1) {
  }
}
