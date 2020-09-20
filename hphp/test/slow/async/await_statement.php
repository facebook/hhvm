<?hh

async function f() {
  print "this takes soooo looong...\n";
}

async function g() {
  await f();
  await f();
  await f();
  $b = await f();
  return $b;
}


<<__EntryPoint>>
function main_await_statement() {
$r = HH\Asio\join(g());
var_dump($r);
}
