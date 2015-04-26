<?hh

function bar($options) {
  var_dump(debug_backtrace($options));
}

function foo($options) {
  HH\set_frame_metadata('I am foo');
  bar($options);
}

foo(0);
foo(DEBUG_BACKTRACE_PROVIDE_METADATA);
