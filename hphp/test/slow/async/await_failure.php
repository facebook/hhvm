<?hh

class Blah extends Exception {}

async function foo() {
  throw new Blah();
}

async function bar() {
  $x = await foo();
  var_dump($x);
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

main();
echo "done\n";
