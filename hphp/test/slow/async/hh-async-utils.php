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
  var_dump(HH\Asio\join(HH\Asio\v($in_v)));
  var_dump(HH\Asio\join(HH\Asio\m($in_v)));

  $in_m = Map {
    '123' => asyncval('456'),
    'herp' => asyncval('derp'),
  };
  var_dump(HH\Asio\join(HH\Asio\v($in_m)));
  var_dump(HH\Asio\join(HH\Asio\m($in_m)));
}

main();
