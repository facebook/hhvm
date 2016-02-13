<?hh

function testCycleBackTrace() {
  global $aawh;
  try {
    $aawh = AwaitAllWaitHandle::fromArray(array(
      async {
        // asio checks first unfinished child
        await RescheduleWaitHandle::create(0, 2);
      },
      async {
        global $aawh;
        await RescheduleWaitHandle::create(0, 0);
        $bt = debug_backtrace();
        var_dump(array_map($frame ==> $frame['function'], $bt));
        // Form a cycle: await parent
        await $aawh;
      },
      async {
        await RescheduleWaitHandle::create(0, 1);
        // Backtracing while having cycle
        $bt = debug_backtrace();
        var_dump(array_map($frame ==> $frame['function'], $bt));
      }
    ));
    HH\Asio\join($aawh);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

testCycleBackTrace();
