<?hh
<<file: __EnableUnstableFeatures('pipe_await')>>

async function bar1(): Awaitable<int> {
  echo "bar1()\n";
  return 1;
}

async function bar2(int $x): Awaitable<int> {
  echo "bar2()\n";
  return $x + 1;
}

function f(int $x): int {
  echo "f($x)\n";
  return $x + 1;
}

async function async_main(): Awaitable<void> {
  echo "--- a\n";
  var_dump((await bar1()) |> f($$) |> await bar2($$));
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
