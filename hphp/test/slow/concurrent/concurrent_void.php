<?hh

async function foo1(): Awaitable<void> {
  concurrent {
    $x = await genx();
    await geny();
    $z = await genz();
  }
  var_dump($x);
  var_dump($z);
}

async function genx(): Awaitable<int> { return 42; }
async function geny(): Awaitable<void> {}
async function genz(): Awaitable<int> { return 43; }

<<__EntryPoint>>
function main() :mixed{
  \HH\Asio\join(foo1());
}
