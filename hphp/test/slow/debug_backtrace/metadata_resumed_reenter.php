<?hh

async function block() :Awaitable<mixed>{
  await RescheduleWaitHandle::create(0, 0);
}

async function baz($options) :Awaitable<mixed>{
  await block();
  var_dump(debug_backtrace($options));
}

async function bar($options) :Awaitable<mixed>{
  HH\set_frame_metadata('I am bar');
  HH\Asio\join(baz($options));
}

async function foo($options) :Awaitable<mixed>{
  await block();
  await bar($options);
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  await foo(0);
  await foo(DEBUG_BACKTRACE_PROVIDE_METADATA);
}
