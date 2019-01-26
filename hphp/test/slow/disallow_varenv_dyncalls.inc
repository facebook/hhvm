<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function custom_handler($error, $errmsg, $errfile, $errline, $errcontext) {
  $file = substr(strrchr($errfile, '/'), 1);
  echo "$errmsg in $file at line $errline\n";
}
set_error_handler('custom_handler');

function test_compact_dyncall() {
  $var1 = 100;
  $var2 = 200;
  $f = 'compact';
  var_dump($f(['var1', 'var2']));
}

function test_compact_cuf() {
  $var1 = 100;
  $var2 = 200;
  $f = 'compact';
  var_dump(call_user_func($f, ['var1', 'var2']));
}

function test_compact_cuf_array() {
  $var1 = 100;
  $var2 = 200;
  $f = 'compact';
  var_dump(call_user_func_array($f, ['var1', 'var2']));
}

function test_compact_map() {
  $var1 = 100;
  $var2 = 200;
  $f = 'compact';
  var_dump(array_map($f, [['var1', 'var2']]));
}

function test_compact_errors() {
  $f = 'compact';
  $f();
  call_user_func($f);
  call_user_func_array($f, []);
}

function test_getargs_dyncall_impl($a, $b, $c) {
  $f = 'func_get_args';
  var_dump($f());
}
function test_getargs_dyncall() {
  test_getargs_dyncall_impl(100, 'abc', false);
}

function test_getargs_cuf_impl($a, $b, $c) {
  $f = 'func_get_args';
  var_dump(call_user_func($f));
}
function test_getargs_cuf() {
  test_getargs_cuf_impl(100, 'abc', false);
}

function test_getargs_cuf_array_impl($a, $b, $c) {
  $f = 'func_get_args';
  var_dump(call_user_func_array($f, []));
}
function test_getargs_cuf_array() {
  test_getargs_cuf_array_impl(100, 'abc', false);
}

function test_getargs_map_impl($a, $b, $c) {
  $f = 'func_get_args';
  var_dump(array_map($f, ['a']));
}
function test_getargs_map() {
  test_getargs_map_impl(100, 'abc', false);
}

function test_getargs_errors() {
  $f = 'func_get_args';
  $f('abc', 100, false);
  array_map($f, [1]);
}

function get_current_test($all) {
  $tests = apc_fetch('tests');
  if (!is_array($tests)) {
    $tests = [];
    foreach ($all as $t) {
      $tests[] = [$t, 1];
      $tests[] = [$t, 2];
    }
  }
  if (empty($tests)) return null;
  $current = array_shift(&$tests);
  apc_store('tests', $tests);
  return $current;
}

function run_tests() {
  $all = [
    'test_compact_dyncall',
    'test_compact_cuf',
    'test_compact_cuf_array',
    'test_compact_map',
    'test_compact_errors',

    'test_getargs_dyncall',
    'test_getargs_cuf',
    'test_getargs_cuf_array',
    'test_getargs_errors',
  ];

  $test = get_current_test($all);
  if (is_null($test)) return;
  $func = $test[0];
  $iter = $test[1];
  echo "Running $func() #$iter....\n";
  $func();
  echo "$func() done\n";
}
