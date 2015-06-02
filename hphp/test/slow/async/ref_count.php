<?hh

function bar() {
  return SleepWaitHandle::create(1000);
}

async function foo($uid) {
  static $result_cache = array();
  if (!isset($result_cache[$uid])) {
    $result_cache[$uid] = bar();
  }
  return await $result_cache[$uid];
}

$i = 0;
while ($i++ < 15) {
  HH\Asio\join(foo(0));
}
