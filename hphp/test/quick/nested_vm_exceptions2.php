<?php

// Similar to nested_vm_exceptions, except throw intercepted functions
// into it also.

function error_handler() {
  echo "Error handler\n";
  throw new Exception("unhandled exception");
}
set_error_handler('error_handler');

function unary_function($x) {
  // Raise a warning and throw from
  // the user error handler.
  return UNDEFINED === $x;
}

function binary_function($x, $y) {}

fb_intercept('binary_function', 'unary_function', 'unary_function');

try {
  call_user_func_array('binary_function', array(12));
} catch (Exception $x) {
  echo "We hit our handler.\n";
  throw new Exception("Sup");
}

// Try it with no catch also.
call_user_func_array('binary_function', array(12));
