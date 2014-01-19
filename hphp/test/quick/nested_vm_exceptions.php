<?php

// Tests a case where prepareFuncEntry throws in a nested VM context.

function error_handler() {
  echo "Error handler\n";
  throw new Exception("unhandled exception");
}
set_error_handler('error_handler');

function binary_function($x, $y) {
}

try {
  // Throw from the user error handler after raising a warning about
  // arg count.
  call_user_func_array('binary_function', array(12));
} catch (Exception $x) {
  echo "We hit our handler.\n";
  throw new Exception("Sup");
}

// Try it with no catch also.
call_user_func_array('binary_function', array(12));
