<?hh

async function f(): Awaitable<int> {
  return 1;
}

async function async_main(): Awaitable<void> {
  $x = f();
  echo "--- a\n";
  var_dump ($x |> (await $$));
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
