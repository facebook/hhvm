<?hh

function bar($options) {
  var_dump(debug_backtrace($options));
}

function foo($fn, $options) {
  call_user_func($fn, 'I am foo');
  bar($options);
}

foo('HH\set_frame_metadata', 0);
foo('HH\set_frame_metadata', DEBUG_BACKTRACE_PROVIDE_METADATA);
