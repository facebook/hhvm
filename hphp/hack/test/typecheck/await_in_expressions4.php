<?hh

async function f1(Awaitable<int> $a): Awaitable<void> {
  // currently not an error, but should be
  $c = (await $a);
}
