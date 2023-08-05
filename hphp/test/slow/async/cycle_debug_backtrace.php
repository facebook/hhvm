<?hh

class Ref {
  public $value;
}

<<__EntryPoint>>
async function testCycleBackTrace(): Awaitable<void> {
  Exception::setTraceOptions(DEBUG_BACKTRACE_IGNORE_ARGS);

  $ref = new Ref();
  $ref->value = AwaitAllWaitHandle::fromVec(vec[
    async {
      // asio checks first unfinished child
      await RescheduleWaitHandle::create(0, 3);
    },
    $afwh = async {
      await RescheduleWaitHandle::create(0, 0);
      $bt = debug_backtrace();
      var_dump(HH\Lib\Vec\map($bt, $frame ==> $frame['function']));
      // Form a cycle: await parent
      await $ref->value;
    },
    async {
      await RescheduleWaitHandle::create(0, 2);
      // Backtracing while having cycle
      $bt = debug_backtrace();
      var_dump(HH\Lib\Vec\map($bt, $frame ==> $frame['function']));
    }
  ]);

  await RescheduleWaitHandle::create(0, 1);

  try {
    await $ref->value;
  } catch (Exception $e) {
    var_dump($e->getMessage());
    var_dump($e->getTrace());
  }

  try {
    await $afwh;
  } catch (Exception $e) {
    var_dump($e->getMessage());
    var_dump($e->getTrace());
  }
}
