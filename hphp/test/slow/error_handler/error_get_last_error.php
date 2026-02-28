<?hh

function handleError($errno, $errstr, $errfile, $errline) :mixed{
  echo "$errstr\n";
  // error_get_last() didn't available in handler
  var_dump(error_get_last());
}

function shutdownFunc() :mixed{
  var_dump(error_get_last()['line']);
}
<<__EntryPoint>>
function entrypoint_error_get_last_error(): void {

  // at the start it should be NULL
  var_dump(error_get_last());

  set_error_handler(handleError<>);
  register_shutdown_function(shutdownFunc<>);

  try {
    if ($x) {
      echo "x\n";
    } else {
      echo "no x\n";
    }
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }

  // it should been clean
  var_dump(error_get_last());

  nosuchfunc();
}
