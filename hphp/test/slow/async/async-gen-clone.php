<?hh

async function foo() { yield 1; yield 2; yield 3; }

async function test() {
  $f = foo();
  var_dump($f);
  $f2 = clone($f); // should fatal
}

\HH\Asio\join(test());
echo "survived\n";
