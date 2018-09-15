<?hh // strict

async function f(Awaitable<int> $a): Awaitable<int> {
  await $a;
  $b = await $a;
  return await $a;
}
