<?hh

function bar($options) {
  var_dump(debug_backtrace($options));
}

function foo($local, $options) {
  $$local = 'I am foo';
  bar($options);
}

foo('__not_metadata', 0);
foo('__not_metadata', DEBUG_BACKTRACE_PROVIDE_METADATA);
foo('86metadata', 0);
foo('86metadata', DEBUG_BACKTRACE_PROVIDE_METADATA);
