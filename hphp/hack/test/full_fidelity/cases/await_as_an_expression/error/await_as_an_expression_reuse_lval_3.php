<?hh

async function foo(): Awaitable<void> {
  $x = (await genx($x = 42, $x));
}
