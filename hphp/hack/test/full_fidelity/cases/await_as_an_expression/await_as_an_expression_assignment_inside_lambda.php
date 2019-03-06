<?hh

async function foo(): Awaitable<void> {
  $x = await async { $x = 42; };
  $x = await async { if ($x = 42) {} };
}
