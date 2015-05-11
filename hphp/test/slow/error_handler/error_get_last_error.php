<?php

// at the start it should be NULL
var_dump(error_get_last());

function handleError($errno, $errstr, $errfile, $errline) {
  echo "$errstr\n";
  // error_get_last() didn't available in handler
  var_dump(error_get_last());
}

function shutdownFunc() {
  var_dump(error_get_last()['line']);
}

set_error_handler('handleError');
register_shutdown_function('shutdownFunc');

if ($x) {
  echo "x\n";
} else {
  echo "no x\n";
}

// it should been clean
var_dump(error_get_last());

nosuchfunc();
