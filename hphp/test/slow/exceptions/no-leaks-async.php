<?hh

class FooException extends Exception {
  public string $a;
  function __construct() {
    $str = ' ';
    for ($i = 0; $i < 20; ++$i) $str.= $str;
    $this->a = $str;
  }
}

async function f() :Awaitable<mixed>{
  await RescheduleWaitHandle::create(0, 0); // simulate blocking I/O
  throw new FooException();
}

<<__NEVER_INLINE>>
async function run() :Awaitable<mixed>{
  $delta = 0;
  for ($i = 0; $i < 10; $i++) {
    $start = memory_get_usage(true);
    try { await f(); } catch (Exception $e) { $e = null; }
    $delta += (memory_get_usage(true) - $start);
  }
  return $delta;
}


<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  await run(); await run(); await run(); // ignore
  $delta1 = abs(await run());
  $delta2 = abs(await run());
  if ($delta1 + $delta2 > $delta1 * 1.1 + 100000) {
    echo "Memory leak!!\n";
  } else {
    echo "ok!\n";
  }
}
