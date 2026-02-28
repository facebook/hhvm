<?hh

async function genx(int $y): Awaitable<int> { return $y; }

async function foo(): Awaitable<int> {
  concurrent {
    $x = await genx($a1 = 42);
    $y = await genx($a1 + $a2);
    $z = await genx($a2 = 43);
  }
  return $x + $y + $z;
}
