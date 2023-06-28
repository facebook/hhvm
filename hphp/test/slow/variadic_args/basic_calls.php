<?hh

function test_standard_calls() :mixed{
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

function test_standard_insuffient_calls() :mixed{
  echo '========= ', __FUNCTION__, ' ==========', "\n";
  variadic_only_no_vv();
  variadic_only();
  try { variadic_some(); } catch (Exception $e) { var_dump($e->getMessage()); }
  variadic_hack_only();
  try { variadic_hack_some(); } catch (Exception $e) { var_dump($e->getMessage()); }
  echo "\n", '========= static methods ==========', "\n";
  C::st_variadic_only();
  try { C::st_variadic_some(); } catch (Exception $e) { var_dump($e->getMessage()); }
  C::st_variadic_hack_only();
  try { C::st_variadic_hack_some(); } catch (Exception $e) { var_dump($e->getMessage()); }
  echo "\n", '========= instance methods ==========', "\n";
  $inst = new C();
  $inst->variadic_only();
  try { $inst->variadic_some(); } catch (Exception $e) { var_dump($e->getMessage()); }
  $inst->variadic_hack_only();
  try { $inst->variadic_hack_some(); } catch (Exception $e) { var_dump($e->getMessage()); }
  echo "\n\n";
}

function test_single_var_param() :mixed{
  variadic_only_no_vv('a');
}

function main() :mixed{
  test_single_var_param();
  test_standard_calls();
  test_standard_insuffient_calls();
}


<<__EntryPoint>>
function main_basic_calls() :mixed{
error_reporting(-1);
require_once __DIR__.'/variadic_funcs.inc';

main();
}
