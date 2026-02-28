<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function backtrace_contains(varray $bt, string $fn_name): bool {
  foreach ($bt as $frame) {
    if (idx($frame, 'function') === $fn_name) {
      return true;
    }
  }
  return false;
}

function reschedule($priority = 0) :mixed{
  $queue = RescheduleWaitHandle::QUEUE_DEFAULT;
  return RescheduleWaitHandle::create($queue, $priority);
}

async function gen1(): AsyncIterator<int> {
  $bt = debug_backtrace();
  var_dump(backtrace_contains($bt, 'gen1'));
  var_dump(backtrace_contains($bt, 'testAsyncGeneratorBacktrace'));
  await reschedule();
  $bt = debug_backtrace();
  var_dump(backtrace_contains($bt, 'gen1'));
  var_dump(backtrace_contains($bt, 'testAsyncGeneratorBacktrace'));
  yield 1;
  $bt = debug_backtrace();
  var_dump(backtrace_contains($bt, 'gen1'));
  var_dump(backtrace_contains($bt, 'testAsyncGeneratorBacktrace'));
  await reschedule();
  $bt = debug_backtrace();
  var_dump(backtrace_contains($bt, 'gen1'));
  var_dump(backtrace_contains($bt, 'testAsyncGeneratorBacktrace'));
  yield 2;
  $bt = debug_backtrace();
  var_dump(backtrace_contains($bt, 'gen1'));
  var_dump(backtrace_contains($bt, 'testAsyncGeneratorBacktrace'));
  await reschedule();
  $bt = debug_backtrace();
  var_dump(backtrace_contains($bt, 'gen1'));
  var_dump(backtrace_contains($bt, 'testAsyncGeneratorBacktrace'));
}

async function foo(): Awaitable<void> {
  $bt = debug_backtrace();
  var_dump(backtrace_contains($bt, 'foo'));
  var_dump(backtrace_contains($bt, 'gen2'));
  var_dump(backtrace_contains($bt, 'testAsyncGeneratorBacktrace'));
  await reschedule();
  var_dump(backtrace_contains($bt, 'foo'));
  var_dump(backtrace_contains($bt, 'gen2'));
  var_dump(backtrace_contains($bt, 'testAsyncGeneratorBacktrace'));
}

async function gen2(): AsyncIterator<void> {
  yield;
  await foo();
  await reschedule();
  await foo();
}

async function testAsyncGeneratorBacktrace(): Awaitable<void> {
  echo "=== gen1 ===\n";
  foreach (gen1() await as $val) {
    echo "yield $val\n";
  }
  echo "=== gen2 ===\n";
  foreach (gen2() await as $val) {
  }
}


<<__EntryPoint>>
function main_backtrace_async_generators() :mixed{
HH\Asio\join(testAsyncGeneratorBacktrace());
}
