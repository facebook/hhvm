<?hh

class Blah extends Exception {}

async function foo() :Awaitable<mixed>{
  throw new Blah();
}

async function bar() :Awaitable<mixed>{
  try {
    $x = await foo();
    var_dump($x);
  } catch (Exception $e) {
    echo "rethrowing\n";
    throw $e;
  }
}

function main() :mixed{
  $x = bar();

  try {
    var_dump(HH\Asio\join($x));
  } catch (Blah $x) {
    echo "caught exception (join)\n";
  }

  $x = bar();
  if (!$x->isFinished()) { exit("test failed"); }
  try {
    var_dump(HH\Asio\result($x));
  } catch (Blah $x) {
    echo "caught exception (result)\n";
  }
}


<<__EntryPoint>>
function main_await_failure() :mixed{
main();
echo "done\n";
}
