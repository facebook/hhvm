<?hh

async function f() {
  print "async call...\n";
  return 42;
}

async function g() {
  try {
    print "try\n";
  } finally {
    $b = await f();
    print "finally\n";
  }
  print "end\n";
  return $b;
}

$r = HH\Asio\join(g());
var_dump($r);
