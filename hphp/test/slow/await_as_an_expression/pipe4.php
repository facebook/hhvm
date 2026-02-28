<?hh
<<file: __EnableUnstableFeatures('pipe_await')>>

async function bar1(): Awaitable<int> {
  echo "bar1()\n";
  return 1;
}

function f(int $x): int {
  echo "f($x)\n";
  return $x;
}

async function async_main(): Awaitable<void> {
  echo "--- a\n";
  // PROBLEM (T45921282): The docs say this should be allowed!
  var_dump((await bar1()) |> (f($$) + await bar1()));
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
