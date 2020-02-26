<?hh

// Tests a case where prepareFuncEntry throws in a nested VM context.

function error_handler() {
  echo "Error handler\n";
  throw new Exception("unhandled exception");
}
function binary_function(string $x, $y) {
}
<<__EntryPoint>> function main(): void {
set_error_handler(fun('error_handler'));

try {
  // Throw from the user error handler after raising a warning about
  // arg count.
  call_user_func_array(fun('binary_function'), varray[12, 12]);
} catch (Exception $x) {
  echo "We hit our handler.\n";
  throw new Exception("Sup");
}

// Try it with no catch also.
call_user_func_array(fun('binary_function'), varray[12, 12]);
}
