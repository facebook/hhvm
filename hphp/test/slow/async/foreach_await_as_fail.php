<?hh

function generator() :AsyncGenerator<mixed,mixed,void>{
  yield 42;
}

async function foo() :Awaitable<mixed>{
  foreach (generator() await as $value) {
    echo "$value\n";
  }
}


<<__EntryPoint>>
function main_foreach_await_as_fail() :mixed{
HH\Asio\join(foo());
}
