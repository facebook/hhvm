<?hh

async function bar($options) {
  await RescheduleWaitHandle::create(0, 0);
  var_dump(debug_backtrace($options));
}

async function foo($options) {
  HH\set_frame_metadata('I am foo');
  await bar($options);
}

HH\Asio\join(foo(0));
HH\Asio\join(foo(DEBUG_BACKTRACE_PROVIDE_METADATA));
