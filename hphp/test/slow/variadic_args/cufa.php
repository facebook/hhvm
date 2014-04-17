<?hh

error_reporting(-1);
require_once __DIR__.'/variadic_funcs.inc';

function test_cufa($args) {
  echo "\n", '********* ', __FUNCTION__, ' **********', "\n";
  var_dump($args);

  echo "\n", '========= functions ==========', "\n";
  call_user_func_array('variadic_only_no_vv', $args);
  call_user_func_array('variadic_only', $args);
  call_user_func_array('variadic_some', $args);
  call_user_func_array('variadic_hack_only', $args);
  call_user_func_array('variadic_hack_some', $args);
  echo "\n", '========= static methods ==========', "\n";
  call_user_func_array(['C', 'st_variadic_only'], $args);
  call_user_func_array(['C', 'st_variadic_some'], $args);
  call_user_func_array(['C', 'st_variadic_hack_only'], $args);
  call_user_func_array(['C', 'st_variadic_hack_some'], $args);
  echo "\n", '========= instance methods ==========', "\n";
  $inst = new C();
  call_user_func_array([$inst, 'variadic_only'], $args);
  call_user_func_array([$inst, 'variadic_some'], $args);
  call_user_func_array([$inst, 'variadic_hack_only'], $args);
  call_user_func_array([$inst, 'variadic_hack_some'], $args);
}

function test_cuf() {
  echo "\n", '********* ', __FUNCTION__, ' **********', "\n";
  call_user_func('variadic_only_no_vv', 'a', 'b', 'c');
  call_user_func('variadic_only', 'a', 'b', 'c');
  call_user_func('variadic_some', 'a', 'b', 'c');
  call_user_func('variadic_hack_only', 'a', 'b', 'c');
  call_user_func('variadic_hack_some', 'a', 'b', 'c');
  echo "\n", '========= static methods ==========', "\n";
  call_user_func(['C', 'st_variadic_only'], 'a', 'b', 'c');
  call_user_func(['C', 'st_variadic_some'], 'a', 'b', 'c');
  call_user_func(['C', 'st_variadic_hack_only'], 'a', 'b', 'c');
  call_user_func(['C', 'st_variadic_hack_some'], 'a', 'b', 'c');
  echo "\n", '========= instance methods ==========', "\n";
  $inst = new C();
  call_user_func([$inst, 'variadic_only'], 'a', 'b', 'c');
  call_user_func([$inst, 'variadic_some'], 'a', 'b', 'c');
  call_user_func([$inst, 'variadic_hack_only'], 'a', 'b', 'c');
  call_user_func([$inst, 'variadic_hack_some'], 'a', 'b', 'c');
}

function test_cuf_insuffient_calls() {
  echo "\n", '********* ', __FUNCTION__, ' **********', "\n";
  call_user_func('variadic_only_no_vv');
  call_user_func('variadic_only');
  call_user_func('variadic_some');
  call_user_func('variadic_hack_only');
  call_user_func('variadic_hack_some');
  echo "\n", '========= static methods ==========', "\n";
  call_user_func(['C', 'st_variadic_only']);
  call_user_func(['C', 'st_variadic_some']);
  call_user_func(['C', 'st_variadic_hack_only']);
  call_user_func(['C', 'st_variadic_hack_some']);
  echo "\n", '========= instance methods ==========', "\n";
  $inst = new C();
  call_user_func([$inst, 'variadic_only']);
  call_user_func([$inst, 'variadic_some']);
  call_user_func([$inst, 'variadic_hack_only']);
  call_user_func([$inst, 'variadic_hack_some']);
}

function test_stack_should_not_overflow(...$args) {
  echo "\n", '********* ', __FUNCTION__, ' **********', "\n";
  var_dump(is_array($args));
  var_dump(count($args));
}

function main() {
  test_cuf();
  test_cuf_insuffient_calls();

  test_cufa(array('a', 'b', 'c'));
  test_cufa(array('a'));
  test_cufa(array());

  call_user_func_array('test_stack_should_not_overflow', range(0, 10000));
  call_user_func_array('test_stack_should_not_overflow', range(0, 30000));
}
main();
