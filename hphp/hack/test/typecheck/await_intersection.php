<?hh

async function test(?Awaitable<mixed> $x): Awaitable<void> {
  if ($x !== null) {
    await $x;
  }
}
