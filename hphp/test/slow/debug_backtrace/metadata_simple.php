<?hh

function bar($options) :mixed{
  var_dump(debug_backtrace($options));
}

function foo($options) :mixed{
  HH\set_frame_metadata('I am foo');
  bar($options);
}

<<__EntryPoint>>
function main(): void {
  foo(0);
  foo(DEBUG_BACKTRACE_PROVIDE_METADATA);
}
