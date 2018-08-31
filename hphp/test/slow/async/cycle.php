<?hh

async function bar($a) {
  return $a;
}

async function foo() {
  for ($i = 0; $i < 42; $i++) {
    $b = await bar($i);
    var_dump($b);
  }
  return "finished!";
}



<<__EntryPoint>>
function main_cycle() {
var_dump(HH\Asio\join(foo()));
}
