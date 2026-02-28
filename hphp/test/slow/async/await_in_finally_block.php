<?hh

async function f() :Awaitable<mixed>{
  print "async call...\n";
  return 42;
}

async function g() :Awaitable<mixed>{
  try {
    print "try\n";
  } finally {
    $b = await f();
    print "finally\n";
  }
  print "end\n";
  return $b;
}


<<__EntryPoint>>
function main_await_in_finally_block() :mixed{
$r = HH\Asio\join(g());
var_dump($r);
}
