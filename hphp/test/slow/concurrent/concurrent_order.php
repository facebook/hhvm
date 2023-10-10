<?hh

function add4(int $x1, int $x2, int $x3, int $x4): int {
  return $x1 + $x2 + $x3 + $x4;
}

async function foo1(): Awaitable<void> {
  $vec = vec[];
  $vec[] = show(1) + show(2);
  $vec[] = add4(show(5), (await gen_id(show(3))), show(6), (await gen_id(show(4))));
  var_dump('start concurrent');
  concurrent {
    $vec[] = add4(show(7), (await gen_id(show(1))), show(8), (await gen_id(show(2))));
    $vec[] = add4(show(9), (await gen_id(show(3))), show(10), (await gen_id(show(4))));
    $vec[] = show(11) + (await gen_id(show(5))) + show(12) + (await gen_id(show(6)));
  }
  var_dump($vec);
}

async function gen_id<T>(T $x): Awaitable<T> {
  return $x;
}

function show(int $x): int {
  var_dump($x);
  return $x;
}

<<__EntryPoint>>
function main() :mixed{
  \HH\Asio\join(foo1());
}
