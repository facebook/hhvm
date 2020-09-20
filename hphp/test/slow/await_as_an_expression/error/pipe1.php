<?hh

async function async_main(): Awaitable<void> {
  echo "--- a\n";
  (await $x) |> (await $$);
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
