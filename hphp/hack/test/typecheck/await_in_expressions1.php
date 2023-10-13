<?hh

async function f1(Awaitable<int> $a): Awaitable<void> {
  $x = (await $a) + 1;
}
