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
  // PROBLEM (T45921282): The docs say this should be allowed!
  (await bar1()) |> (f($$) + await bar1());
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
