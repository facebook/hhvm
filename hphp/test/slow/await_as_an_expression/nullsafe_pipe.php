<?hh

async function f(): Awaitable<int> {
  return 1;
}

async function g(): Awaitable<Awaitable<int>> {
  return f();
}

async function h(): Awaitable<?Awaitable<int>> {
  return null;
}

async function async_main(): Awaitable<void> {
  $x = g();
  $y = h();
  echo "--- a\n";
  var_dump((await $x) |?> (await $$));
  var_dump((await $y) |?> (await $$));
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
