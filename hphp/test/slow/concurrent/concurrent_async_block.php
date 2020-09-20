<?hh // strict

async function foo1(): Awaitable<void> {
  await async {};
  concurrent {
    await async {};
    await async {};
  }
  await async {};
}

<<__EntryPoint>>
function main() {
  \HH\Asio\join(foo1());
}
