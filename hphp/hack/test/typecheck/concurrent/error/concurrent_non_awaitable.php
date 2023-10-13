<?hh

function get(): int { return 42; }
async function gen(): Awaitable<int> { return 42; }

async function foo(): Awaitable<void> {
  concurrent {
    $x = await get();
    $y = await gen();
  }
}
