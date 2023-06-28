<?hh

class Ref {
  public $value;
}

function testCycleBackTrace() :mixed{
  Exception::setTraceOptions(DEBUG_BACKTRACE_IGNORE_ARGS);

  $ref = new Ref();
  $ref->value = AwaitAllWaitHandle::fromVec(vec[
    async {
      // asio checks first unfinished child
      await RescheduleWaitHandle::create(0, 2);
    },
    $afwh = async {
      await RescheduleWaitHandle::create(0, 0);
      $bt = debug_backtrace();
      var_dump(array_map($frame ==> $frame['function'], $bt));
      // Form a cycle: await parent
      await $ref->value;
    },
    async {
      await RescheduleWaitHandle::create(0, 1);
      // Backtracing while having cycle
      $bt = debug_backtrace();
      var_dump(array_map($frame ==> $frame['function'], $bt));
    }
  ]);

  try {
    HH\Asio\join($ref->value);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    HH\Asio\join($afwh);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
<<__EntryPoint>>
function entrypoint_cycle_debug_backtrace(): void {

  testCycleBackTrace();
}
