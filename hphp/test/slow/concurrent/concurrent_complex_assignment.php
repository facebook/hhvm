<?hh

async function foo1(): Awaitable<void> {
  $obj = new stdClass();
  $obj_alias = $obj;
  $vec = vec[];
  concurrent {
    $obj->prop1 = await gen_id(1);
    $obj_alias->prop1 = await gen_id(2);
    $vec[] = await gen_id(3);
    $vec[] = await gen_id(4);
    $obj->prop2 = await gen_id(5);
  }
  var_dump($obj);
  var_dump($vec);
}

async function gen_id<T>(T $x): Awaitable<T> {
  return $x;
}

<<__EntryPoint>>
function main() :mixed{
  \HH\Asio\join(foo1());
}
