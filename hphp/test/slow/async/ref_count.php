<?hh

function bar() :mixed{
  return SleepWaitHandle::create(1000);
}

abstract final class FooStatics {
  public static $result_cache = dict[];
}

async function foo($uid) :Awaitable<mixed>{
  if (!isset(FooStatics::$result_cache[$uid])) {
    FooStatics::$result_cache[$uid] = bar();
  }
  return await FooStatics::$result_cache[$uid];
}


<<__EntryPoint>>
function main_ref_count() :mixed{
$i = 0;
while ($i++ < 15) {
  HH\Asio\join(foo(0));
}
}
