<?hh // strict

async function foo1(): Awaitable<void> {
  $x = (await genx()) + (await geny());
  var_dump($x);
}

async function genx(): Awaitable<int> {
  return 42;
}
function geny(): int {
  return 43;
}

<<__EntryPoint>>
function main() {
  \HH\Asio\join(foo1());
}
