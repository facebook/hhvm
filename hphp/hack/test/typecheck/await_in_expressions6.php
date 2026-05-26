<?hh

async function f1(Awaitable<bool> $a): Awaitable<void> {
  do {
  } while (await $a);
}
