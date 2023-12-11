<?hh

// Similar to nested_vm_exceptions, except throw intercepted functions
// into it also.

function error_handler() :mixed{
  echo "Error handler\n";
  throw new Exception("unhandled exception");
}

function unary_function($_1, $_2, inout $_3) :mixed{
  // Raise a warning and throw from
  // the user error handler.
  trigger_error("raise a notice", E_USER_NOTICE);
  return shape('value' => null);
}

function binary_function(string $x, $y) :mixed{}
<<__EntryPoint>>
function entrypoint_nested_vm_exceptions2(): void {
  set_error_handler(error_handler<>);

  fb_intercept2('binary_function', 'unary_function');

  try {
    call_user_func_array(binary_function<>, vec[12, 12]);
  } catch (Exception $x) {
    echo "We hit our handler.\n";
    throw new Exception("Sup");
  }

  // Try it with no catch also.
  call_user_func_array(binary_function<>, vec[12, 12]);
}
