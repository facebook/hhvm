<?hh

async function bar($a) :Awaitable<mixed>{
  return $a;
}

async function foo() :Awaitable<mixed>{
  for ($i = 0; $i < 42; $i++) {
    $b = await bar($i);
    var_dump($b);
  }
  return "finished!";
}



<<__EntryPoint>>
function main_cycle() :mixed{
var_dump(HH\Asio\join(foo()));
}
