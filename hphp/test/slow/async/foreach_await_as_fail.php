<?hh

function generator() {
  yield 42;
}

async function foo() {
  foreach (generator() await as $value) {
    echo "$value\n";
  }
}


<<__EntryPoint>>
function main_foreach_await_as_fail() {
HH\Asio\join(foo());
}
