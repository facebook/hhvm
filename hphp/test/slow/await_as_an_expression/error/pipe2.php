<?hh

async function async_main(): Awaitable<void> {
  echo "--- a\n";
  // PROBLEM (T45921282): The docs say this should be allowed!
  $x |> (await $$);
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
