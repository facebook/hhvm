<?hh // strict

async function genx(): Awaitable<int> { return 42; }
async function geny(int $y): Awaitable<int> { return $y; }

async function foo(): Awaitable<int> {
  concurrent {
    $x = await genx();
    $y = await geny($x);
  }
  return $x;
}
