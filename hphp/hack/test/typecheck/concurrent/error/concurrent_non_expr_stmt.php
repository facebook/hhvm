<?hh

async function genx(): Awaitable<int> { return 42; }

async function foo(): Awaitable<void> {
  concurrent {
    $x = await genx();
    if (true) {
      $y = await genx();
    }
  }
}
