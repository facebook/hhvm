<?hh

class C {
  public static async function f(): Awaitable<int> {
    return 1;
  }
}

async function async_main(): Awaitable<void> {
  echo "--- a\n";
  var_dump (C::class |> (await $$::f()));
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
