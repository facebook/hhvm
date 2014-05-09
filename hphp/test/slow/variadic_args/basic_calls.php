<?hh

error_reporting(-1);
require_once __DIR__.'/variadic_funcs.inc';

function test_standard_calls() {
  echo '========= ', __FUNCTION__, ' ==========', "\n";
  variadic_only_no_vv('a', 'b', 'c');
  variadic_only('a', 'b', 'c');
  variadic_some('a', 'b', 'c');
  variadic_hack_only('a', 'b', 'c');
  variadic_hack_some('a', 'b', 'c');
  echo "\n", '========= static methods ==========', "\n";
  C::st_variadic_only('a', 'b', 'c');
  C::st_variadic_some('a', 'b', 'c');
  C::st_variadic_hack_only('a', 'b', 'c');
  C::st_variadic_hack_some('a', 'b', 'c');
  echo "\n", '========= instance methods ==========', "\n";
  $inst = new C();
  $inst->variadic_only('a', 'b', 'c');
  $inst->variadic_some('a', 'b', 'c');
  $inst->variadic_hack_only('a', 'b', 'c');
  $inst->variadic_hack_some('a', 'b', 'c');
  echo "\n\n";
}

function test_standard_insuffient_calls() {
  echo '========= ', __FUNCTION__, ' ==========', "\n";
  variadic_only_no_vv();
  variadic_only();
  variadic_some();
  variadic_hack_only();
  variadic_hack_some();
  echo "\n", '========= static methods ==========', "\n";
  C::st_variadic_only();
  C::st_variadic_some();
  C::st_variadic_hack_only();
  C::st_variadic_hack_some();
  echo "\n", '========= instance methods ==========', "\n";
  $inst = new C();
  $inst->variadic_only();
  $inst->variadic_some();
  $inst->variadic_hack_only();
  $inst->variadic_hack_some();
  echo "\n\n";
}

function test_single_var_param() {
  variadic_only_no_vv('a');
}

function main() {
  test_single_var_param();
  test_standard_calls();
  test_standard_insuffient_calls();
}

main();
