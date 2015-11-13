<?hh

async function gen_to_string(int $foo): Awaitable<string> {
  return (string) $foo;
}
async function gen_with_key_to_string(int $k, int $v): Awaitable<string> {
  return $k.' => '.$v;
}

function prep<T>(Awaitable<T> $in): T {
  return $in->getWaitHandle()->join();
}

function main() {
  $v = Vector { 1, 2, 3 };
  $m = Map {
    10 => 100,
    20 => 200,
    30 => 300,
  };
  var_dump(prep(HH\Asio\vm($v, fun('gen_to_string'))));
  var_dump(prep(HH\Asio\vmk($v, fun('gen_with_key_to_string'))));
  var_dump(prep(HH\Asio\mm($m, fun('gen_to_string'))));
  var_dump(prep(HH\Asio\mmk($m, fun('gen_with_key_to_string'))));
}

main();
