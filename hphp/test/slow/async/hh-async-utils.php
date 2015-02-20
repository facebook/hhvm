<?hh

async function asyncval<T>(T $in): Awaitable<T> {
  return $in;
}

function main() {
  $in_v = Vector {
    asyncval(123),
    asyncval(456),
    asyncval(789),
  };
  var_dump(HH\Asio\v($in_v)->getWaitHandle()->join());
  var_dump(HH\Asio\m($in_v)->getWaitHandle()->join());

  $in_m = Map {
    '123' => asyncval('456'),
    'herp' => asyncval('derp'),
  };
  var_dump(HH\Asio\v($in_m)->getWaitHandle()->join());
  var_dump(HH\Asio\m($in_m)->getWaitHandle()->join());
}

main();
