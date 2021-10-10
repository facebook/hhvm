<?hh

async function block() {
  await RescheduleWaitHandle::create(0, 0);
}

async function baz($options) {
  await block();
  var_dump(debug_backtrace($options));
}

async function bar($options) {
  HH\set_frame_metadata('I am bar');
  HH\Asio\join(baz($options));
}

async function foo($options) {
  await block();
  await bar($options);
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  await foo(0);
  await foo(DEBUG_BACKTRACE_PROVIDE_METADATA);
}
