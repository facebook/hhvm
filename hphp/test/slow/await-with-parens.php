<?hh

async function bar(): Awaitable<int> {
  return 1;
}

function foo(): int {
  return (await bar());
}
