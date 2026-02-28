<?hh

class Heh { public function yo() :mixed{ echo "yo\n"; } }

async function bar() :Awaitable<mixed>{ return new Heh(); }
async function foo() :Awaitable<mixed>{
  $x = await bar();
  $x->yo();
  return $x;
}

function main() :mixed{
  $go = foo();
  $blah = HH\Asio\join($go);
  $blah->yo();
}


<<__EntryPoint>>
function main_async_await_001() :mixed{
main();
}
