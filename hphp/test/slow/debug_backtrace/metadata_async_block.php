<?hh

async function bar($options) :Awaitable<mixed>{
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  var_dump(debug_backtrace($options));
}

async function foo($options) :Awaitable<mixed>{
  HH\set_frame_metadata('I am foo');
  await bar($options);
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  await foo(DEBUG_BACKTRACE_IGNORE_ARGS);
  await foo(DEBUG_BACKTRACE_PROVIDE_METADATA | DEBUG_BACKTRACE_IGNORE_ARGS);
}
