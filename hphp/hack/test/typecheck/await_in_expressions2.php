<?hh

async function f1(Awaitable<int> $a): Awaitable<void> {
  $x = 1;
  $b = ($x = await $a);
}
