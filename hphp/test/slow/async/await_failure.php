<?hh

class Blah extends Exception {}

async function foo() {
  throw new Blah();
}

async function bar() {
  try {
    $x = await foo();
    var_dump($x);
  } catch (Exception $e) {
    echo "rethrowing\n";
    throw $e;
  }
}

function main() {
  $x = bar();

  try {
    var_dump(HH\Asio\join($x));
  } catch (Blah $x) {
    echo "caught exception (join)\n";
  }

  $x = bar();
  if (!$x->isFinished()) { die("test failed"); }
  try {
    var_dump(HH\Asio\result($x));
  } catch (Blah $x) {
    echo "caught exception (result)\n";
  }
}


<<__EntryPoint>>
function main_await_failure() {
main();
echo "done\n";
}
