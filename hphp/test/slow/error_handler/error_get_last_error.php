<?hh

function handleError($errno, $errstr, $errfile, $errline) {
  echo "$errstr\n";
  // error_get_last() didn't available in handler
  var_dump(error_get_last());
}

function shutdownFunc() {
  var_dump(error_get_last()['line']);
}
<<__EntryPoint>>
function entrypoint_error_get_last_error(): void {

  // at the start it should be NULL
  var_dump(error_get_last());

  set_error_handler(fun('handleError'));
  register_shutdown_function(fun('shutdownFunc'));

  if ($x) {
    echo "x\n";
  } else {
    echo "no x\n";
  }

  // it should been clean
  var_dump(error_get_last());

  nosuchfunc();
}
