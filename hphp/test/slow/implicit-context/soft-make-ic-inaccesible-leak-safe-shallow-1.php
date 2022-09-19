<?hh

<<__Memoize(#SoftMakeICInaccessible)>>
function soft_ic_inaccessible()[leak_safe_shallow] {
  // We need to observe this trigger_error
  trigger_error("inside");
}

function handler($errno, $errstr, $errfile, $errline,
                 $errcontext, $backtrace, $ic_blame) {
  echo "$errstr\n";
  var_dump($ic_blame);
}

<<__EntryPoint>>
function main() {
  set_error_handler(handler<>);
  soft_ic_inaccessible();
}
