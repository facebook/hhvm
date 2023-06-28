<?hh

async function gen_to_string(int $foo): Awaitable<string> {
  return (string) $foo;
}
async function gen_with_key_to_string(int $k, int $v): Awaitable<string> {
  return $k.' => '.$v;
}

function main() :mixed{
  $v = Vector { 1, 2, 3 };
  $m = Map {
    10 => 100,
    20 => 200,
    30 => 300,
  };
  var_dump(\HH\Asio\join(HH\Asio\vm($v, gen_to_string<>)));
  var_dump(\HH\Asio\join(HH\Asio\vmk($v, gen_with_key_to_string<>)));
  var_dump(\HH\Asio\join(HH\Asio\mm($m, gen_to_string<>)));
  var_dump(\HH\Asio\join(HH\Asio\mmk($m, gen_with_key_to_string<>)));
}


<<__EntryPoint>>
function main_hh_async_mapper_changes_type() :mixed{
main();
}
