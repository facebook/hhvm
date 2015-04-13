<?hh

async function bar($options) {
  var_dump(debug_backtrace($options));
}

async function foo($options) {
  HH\set_frame_metadata('I am foo');
  await bar($options);
}

foo(0)->join();
foo(DEBUG_BACKTRACE_PROVIDE_METADATA)->join();
