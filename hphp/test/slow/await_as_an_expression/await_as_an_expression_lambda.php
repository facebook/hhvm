<?hh

function id<T>(T $x): T { return $x; }

async function foo1(): Awaitable<void> {
  var_dump('0');
  $x = async () ==> id(await async { var_dump('2'); return 42; });
  var_dump('1');
  var_dump(await $x());
}

<<__EntryPoint>>
function main() :mixed{
  \HH\Asio\join(foo1());
}
