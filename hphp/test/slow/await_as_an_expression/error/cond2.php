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
  $x = (print "b\n") + await bar1() + (print "c\n") ? await bar2() : 4;
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
