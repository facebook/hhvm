<?hh // partial

async function test($x): Awaitable<void> {
  if ($x !== null) {
    await $x;
  }
}
