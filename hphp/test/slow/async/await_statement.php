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

$r = g()->join();
var_dump($r);
