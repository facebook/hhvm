<?hh

function bar() {
  return SleepWaitHandle::create(1000);
}

abstract final class FooStatics {
  public static $result_cache = varray[];
}

async function foo($uid) {
  if (!isset(FooStatics::$result_cache[$uid])) {
    FooStatics::$result_cache[$uid] = bar();
  }
  return await FooStatics::$result_cache[$uid];
}


<<__EntryPoint>>
function main_ref_count() {
$i = 0;
while ($i++ < 15) {
  HH\Asio\join(foo(0));
}
}
