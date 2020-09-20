<?hh

async function bar2(): Awaitable<int> {
  echo "bar2()\n";
  return 1;
}

async function async_main(): Awaitable<void> {
  echo "--- a\n";
  await bar2() ??= 1;
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
