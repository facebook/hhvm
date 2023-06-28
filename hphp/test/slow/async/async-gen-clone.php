<?hh

async function foo() :AsyncGenerator<mixed,mixed,void>{ yield 1; yield 2; yield 3; }

async function test() :Awaitable<mixed>{
  $f = foo();
  var_dump($f);
  $f2 = clone($f); // should fatal
}


<<__EntryPoint>>
function main_async_gen_clone() :mixed{
\HH\Asio\join(test());
echo "survived\n";
}
