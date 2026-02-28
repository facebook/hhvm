<?hh

async function f(): Awaitable<int> {
  return 1;
}

async function g(): Awaitable<Awaitable<int>> {
  return f();
}

async function async_main(): Awaitable<void> {
  $x = g();
  echo "--- a\n";
  var_dump((await $x) |> (await $$));
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
