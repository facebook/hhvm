<?hh

async function backtracing_func(): Awaitable<string> {
  foreach (debug_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS) as $frame) {
    if (HH\Lib\C\contains_key($frame, 'class')) {
      printf("%s%s%s\n", $frame['class'], $frame['type'], $frame['function']);
    } else {
      printf("%s\n", $frame['function']);
    }
    if ($frame['function'] === 'main') break;
  }
  return __hhvm_intrinsics\launder_value("Done\n");
}

async function reschedule(): Awaitable<void> {
  return await RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_DEFAULT,
    0,
  );
}

class C {
  public async function foo(): Awaitable<string> {
    return await $this->bar();
  }
  public async function bar(): Awaitable<string> {
    await reschedule();
    return await backtracing_func();
  }
}

<<__EntryPoint>> async function main(): Awaitable<void> {
  echo await (new C())->foo();
}
