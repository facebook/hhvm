<?hh

// Similar to nested_vm_exceptions, except throw intercepted functions
// into it also.

function error_handler() {
  echo "Error handler\n";
  throw new Exception("unhandled exception");
}
set_error_handler(fun('error_handler'));

function unary_function($_1, $_2, inout $_3, $_4, inout $_5) {
  // Raise a warning and throw from
  // the user error handler.
  trigger_error("raise a notice", E_USER_NOTICE);
}

function binary_function(string $x, $y) {}

fb_intercept('binary_function', 'unary_function', 'unary_function');

try {
  call_user_func_array(fun('binary_function'), varray[12, 12]);
} catch (Exception $x) {
  echo "We hit our handler.\n";
  throw new Exception("Sup");
}

// Try it with no catch also.
call_user_func_array(fun('binary_function'), varray[12, 12]);
