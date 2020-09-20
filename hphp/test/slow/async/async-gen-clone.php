<?hh

async function foo() { yield 1; yield 2; yield 3; }

async function test() {
  $f = foo();
  var_dump($f);
  $f2 = clone($f); // should fatal
}


<<__EntryPoint>>
function main_async_gen_clone() {
\HH\Asio\join(test());
echo "survived\n";
}
