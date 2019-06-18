<?hh

async function bar1(): Awaitable<int> {
  echo "bar1()\n";
  return 1;
}

async function async_main(): Awaitable<void> {
  echo "--- a\n";
  $x ??= await bar1();
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
