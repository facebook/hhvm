<?hh

async function bar($options) {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  var_dump(debug_backtrace($options));
}

async function foo($options) {
  HH\set_frame_metadata('I am foo');
  await bar($options);
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  await foo(0);
  await foo(DEBUG_BACKTRACE_PROVIDE_METADATA);
}
