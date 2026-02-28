<?hh

// Tests a case where prepareFuncEntry throws in a nested VM context.

function error_handler() :mixed{
  echo "Error handler\n";
  throw new Exception("unhandled exception");
}
function binary_function(string $x, $y) :mixed{
}
<<__EntryPoint>> function main(): void {
set_error_handler(error_handler<>);

try {
  // Throw from the user error handler after raising a warning about
  // arg count.
  call_user_func_array(binary_function<>, vec[12, 12]);
} catch (Exception $x) {
  echo "We hit our handler.\n";
  throw new Exception("Sup");
}

// Try it with no catch also.
call_user_func_array(binary_function<>, vec[12, 12]);
}
