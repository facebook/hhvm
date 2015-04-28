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
    echo "caught exception\n";
  }
}

main();
echo "done\n";
