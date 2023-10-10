<?hh

async function foo1(): Awaitable<void> {
  await async {};
  concurrent {
    await async {};
    await async {};
  }
  await async {};
}

<<__EntryPoint>>
function main() :mixed{
  \HH\Asio\join(foo1());
}
