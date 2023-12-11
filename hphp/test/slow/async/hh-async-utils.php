<?hh

async function asyncval<T>(T $in): Awaitable<T> {
  return $in;
}

function main() :mixed{
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

  var_dump(HH\Asio\join(HH\Asio\va(
    asyncval(123),
    asyncval(456),
    asyncval(789),
  )));

  var_dump(HH\Asio\join(HH\Asio\va(
    asyncval(123),
    asyncval('foo'),
    asyncval(vec[1]),
  )));
}


<<__EntryPoint>>
function main_hh_async_utils() :mixed{
main();
}
