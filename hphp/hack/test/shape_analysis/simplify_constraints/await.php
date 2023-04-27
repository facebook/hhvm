<?hh

async function f(Awaitable<int> $i): Awaitable<void> {
  await $i;
  dict['a' => 42];
}
