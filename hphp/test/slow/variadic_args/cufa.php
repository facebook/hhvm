<?hh

function test_cufa($args) {
  echo "\n", '********* ', __FUNCTION__, ' **********', "\n";
  var_dump($args);

  echo "\n", '========= functions ==========', "\n";
  call_user_func_array(fun('variadic_only_no_vv'), $args);
  call_user_func_array(fun('variadic_only'), $args);
  try { call_user_func_array(fun('variadic_some'), $args); } catch (Exception $e) { var_dump($e->getMessage()); }
  call_user_func_array(fun('variadic_hack_only'), $args);
  try { call_user_func_array(fun('variadic_hack_some'), $args); } catch (Exception $e) { var_dump($e->getMessage()); }
  echo "\n", '========= static methods ==========', "\n";
  call_user_func_array(varray['C', 'st_variadic_only'], $args);
  try { call_user_func_array(varray['C', 'st_variadic_some'], $args); } catch (Exception $e) { var_dump($e->getMessage()); }
  call_user_func_array(varray['C', 'st_variadic_hack_only'], $args);
  try { call_user_func_array(varray['C', 'st_variadic_hack_some'], $args); } catch (Exception $e) { var_dump($e->getMessage()); }
  echo "\n", '========= instance methods ==========', "\n";
  $inst = new C();
  call_user_func_array(varray[$inst, 'variadic_only'], $args);
  try { call_user_func_array(varray[$inst, 'variadic_some'], $args); } catch (Exception $e) { var_dump($e->getMessage()); }
  call_user_func_array(varray[$inst, 'variadic_hack_only'], $args);
  try { call_user_func_array(varray[$inst, 'variadic_hack_some'], $args); } catch (Exception $e) { var_dump($e->getMessage()); }
}

function test_cuf() {
  echo "\n", '********* ', __FUNCTION__, ' **********', "\n";
  call_user_func(fun('variadic_only_no_vv'), 'a', 'b', 'c');
  call_user_func(fun('variadic_only'), 'a', 'b', 'c');
  call_user_func(fun('variadic_some'), 'a', 'b', 'c');
  call_user_func(fun('variadic_hack_only'), 'a', 'b', 'c');
  call_user_func(fun('variadic_hack_some'), 'a', 'b', 'c');
  echo "\n", '========= static methods ==========', "\n";
  call_user_func(varray['C', 'st_variadic_only'], 'a', 'b', 'c');
  call_user_func(varray['C', 'st_variadic_some'], 'a', 'b', 'c');
  call_user_func(varray['C', 'st_variadic_hack_only'], 'a', 'b', 'c');
  call_user_func(varray['C', 'st_variadic_hack_some'], 'a', 'b', 'c');
  echo "\n", '========= instance methods ==========', "\n";
  $inst = new C();
  call_user_func(varray[$inst, 'variadic_only'], 'a', 'b', 'c');
  call_user_func(varray[$inst, 'variadic_some'], 'a', 'b', 'c');
  call_user_func(varray[$inst, 'variadic_hack_only'], 'a', 'b', 'c');
  call_user_func(varray[$inst, 'variadic_hack_some'], 'a', 'b', 'c');
}

function test_cuf_insuffient_calls() {
  echo "\n", '********* ', __FUNCTION__, ' **********', "\n";
  call_user_func(fun('variadic_only_no_vv'));
  call_user_func(fun('variadic_only'));
  try { call_user_func(fun('variadic_some')); } catch (Exception $e) { var_dump($e->getMessage()); }
  call_user_func(fun('variadic_hack_only'));
  try { call_user_func(fun('variadic_hack_some')); } catch (Exception $e) { var_dump($e->getMessage()); }
  echo "\n", '========= static methods ==========', "\n";
  call_user_func(varray['C', 'st_variadic_only']);
  try { call_user_func(varray['C', 'st_variadic_some']); } catch (Exception $e) { var_dump($e->getMessage()); }
  call_user_func(varray['C', 'st_variadic_hack_only']);
  try { call_user_func(varray['C', 'st_variadic_hack_some']); } catch (Exception $e) { var_dump($e->getMessage()); }
  echo "\n", '========= instance methods ==========', "\n";
  $inst = new C();
  call_user_func(varray[$inst, 'variadic_only']);
  try { call_user_func(varray[$inst, 'variadic_some']); } catch (Exception $e) { var_dump($e->getMessage()); }
  call_user_func(varray[$inst, 'variadic_hack_only']);
  try { call_user_func(varray[$inst, 'variadic_hack_some']); } catch (Exception $e) { var_dump($e->getMessage()); }
}

function test_stack_should_not_overflow(...$args) {
  echo "\n", '********* ', __FUNCTION__, ' **********', "\n";
  var_dump(is_array($args));
  var_dump(count($args));
}

function main() {
  test_cuf();
  test_cuf_insuffient_calls();

  test_cufa(varray['a', 'b', 'c']);
  test_cufa(varray['a']);
  test_cufa(varray[]);

  call_user_func_array(fun('test_stack_should_not_overflow'), range(0, 10000));
  call_user_func_array(fun('test_stack_should_not_overflow'), range(0, 30000));
}


<<__EntryPoint>>
function main_cufa() {
error_reporting(-1);
require_once __DIR__.'/variadic_funcs.inc';
main();
}
