<?hh

function bar($options) :mixed{
  var_dump(debug_backtrace($options));
}

function foo($fn, $options) :mixed{
  call_user_func($fn, 'I am foo');
  bar($options);
}

<<__EntryPoint>>
function main(): void {
  foo('HH\set_frame_metadata', 0);
  foo('HH\set_frame_metadata', DEBUG_BACKTRACE_PROVIDE_METADATA);
}
