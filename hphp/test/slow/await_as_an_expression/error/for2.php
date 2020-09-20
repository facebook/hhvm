<?hh

async function bar1(): Awaitable<int> {
  echo "bar1()\n";
  return 1;
}

async function bar2(): Awaitable<int> {
  echo "bar2()\n";
  return 1;
}

async function async_main(): Awaitable<void> {
  echo "--- a\n";
  for ($x = await bar1(); $x < await bar1(); $x += 1) {
    echo "--- b\n";
    $y = bar2();
    echo "--- c\n";
  }
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
