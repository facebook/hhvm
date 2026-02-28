<?hh

async function f() :Awaitable<mixed>{
  print "this takes soooo looong...\n";
}

async function g() :Awaitable<mixed>{
  await f();
  await f();
  await f();
  $b = await f();
  return $b;
}


<<__EntryPoint>>
function main_await_statement() :mixed{
$r = HH\Asio\join(g());
var_dump($r);
}
