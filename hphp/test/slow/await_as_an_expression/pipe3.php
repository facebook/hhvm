<?hh

async function bar1(): Awaitable<int> {
  echo "bar1()\n";
  return 1;
}

function f(int $x): void {
  echo "f($x)\n";
}

async function async_main(): Awaitable<void> {
  echo "--- a\n";
  (await bar1()) |> (print "b\n") + f($$);
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
