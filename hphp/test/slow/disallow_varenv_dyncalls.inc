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

function test_compact_walk() {
  $var1 = 100;
  $var2 = 200;
  $f = 'compact';
  $args = [['var1', 'var2']];
  array_walk($args, $f);
}

function test_compact_errors() {
  $f = 'compact';
  $f();
  call_user_func($f);
  call_user_func_array($f, []);
}

function test_extract_dyncall() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);

  $f = 'extract';
  $f(['var1' => '101', 'var2' => '201']);
  var_dump($var1, $var2);
}

function test_extract_cuf() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);

  $f = 'extract';
  call_user_func($f, ['var1' => '101', 'var2' => '201']);
  var_dump($var1, $var2);
}

function test_extract_cuf_array() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);

  $f = 'extract';
  call_user_func_array($f, [['var1' => '101', 'var2' => '201']]);
  var_dump($var1, $var2);
}

function test_extract_map() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);

  $f = 'extract';
  array_map($f, [['var1' => '101', 'var2' => '201']]);
  var_dump($var1, $var2);
}

function test_extract_walk() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);

  $f = 'extract';
  $args = [['var1' => '101', 'var2' => '201']];
  array_walk($args, $f);
  var_dump($var1, $var2);
}

function test_extract_errors() {
  $f = 'extract';
  $f();
  $f(123);
  $f('foo', 'foo', 'foo', 'foo');
  array_map($f, ['foo']);
  $args = ['foo'];
  array_walk($args, $f);
}

function make_parse_str_str($v1, $v2) {
  return "var1=$v1&var2=$v2";
}

function test_parse_str_dyncall() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);

  $f = 'parse_str';
  $f(make_parse_str_str(101, 201));
  var_dump($var1, $var2);
}

function test_parse_str_cuf() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);

  $f = 'parse_str';
  call_user_func($f, make_parse_str_str(101, 201));
  var_dump($var1, $var2);
}

function test_parse_str_cuf_array() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);

  $f = 'parse_str';
  call_user_func_array($f, [make_parse_str_str(101, 201)]);
  var_dump($var1, $var2);
}

function test_parse_str_map() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);

  $f = 'parse_str';
  array_map($f, [make_parse_str_str(101, 201)]);
  var_dump($var1, $var2);
}

function test_parse_str_walk() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);

  $f = 'parse_str';
  $args = [make_parse_str_str(101, 201)];
  array_walk($args, $f);
  var_dump($var1, $var2);
}

function test_parse_str_errors() {
  $f = 'parse_str';
  $f();
  $f([]);
  array_map($f, [[]]);
}

function test_framemeta_dyncall() {
  $v = '86metadata';
  $$v = 100;
  var_dump($$v);
  $f = 'HH\set_frame_metadata';
  $f('metadata');
  var_dump($$v);
}

function test_framemeta_cuf() {
  $v = '86metadata';
  $$v = 100;
  var_dump($$v);
  $f = 'HH\set_frame_metadata';
  call_user_func($f, 'metadata');
  var_dump($$v);
}

function test_framemeta_cuf_array() {
  $v = '86metadata';
  $$v = 100;
  var_dump($$v);
  $f = 'HH\set_frame_metadata';
  call_user_func_array($f, ['metadata']);
  var_dump($$v);
}

function test_framemeta_map() {
  $v = '86metadata';
  $$v = 100;
  var_dump($$v);
  $f = 'HH\set_frame_metadata';
  array_map($f, ['metadata']);
  var_dump($$v);
}

function test_framemeta_errors() {
  $f = 'HH\set_frame_metadata';
  $f();
  $f('abc', 'def', 'ghi');
  $args = ['foo'];
  array_walk($args, $f);
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
  $args = [1];
  array_walk($args, $f);
}

function test_assert_dyncall_pass() {
  $var = 100;
  var_dump($var);
  $f = 'assert';
  $f('($var = \'200\') && true', 'assertion failed');
  var_dump($var);
}

function test_assert_dyncall_pass2() {
  $f = 'assert';
  $f(true, 'assertion failed');
}

function test_assert_dyncall_fail() {
  $var = 100;
  var_dump($var);
  $f = 'assert';
  $f('($var = \'200\') && false', 'assertion failed');
  var_dump($var);
}

function test_assert_dyncall_fail2() {
  $f = 'assert';
  $f(false, 'assertion failed');
}

function test_assert_cuf_pass() {
  $var = 100;
  var_dump($var);
  $f = 'assert';
  call_user_func($f, '($var = \'200\') && true', 'assertion failed');
  var_dump($var);
}

function test_assert_cuf_pass2() {
  $f = 'assert';
  call_user_func($f, true, 'assertion failed');
}

function test_assert_cuf_fail() {
  $var = 100;
  var_dump($var);
  $f = 'assert';
  call_user_func($f, '($var = \'200\') && false', 'assertion failed');
  var_dump($var);
}

function test_assert_cuf_fail2() {
  $f = 'assert';
  call_user_func($f, false, 'assertion failed');
}

function test_assert_cuf_array_pass() {
  $var = 100;
  var_dump($var);
  $f = 'assert';
  call_user_func_array($f, ['($var = \'200\') && true', 'assertion failed']);
  var_dump($var);
}

function test_assert_cuf_array_pass2() {
  $f = 'assert';
  call_user_func_array($f, [true, 'assertion failed']);
}

function test_assert_cuf_array_fail() {
  $var = 100;
  var_dump($var);
  $f = 'assert';
  call_user_func_array($f, ['($var = \'200\') && false', 'assertion failed']);
  var_dump($var);
}

function test_assert_cuf_array_fail2() {
  $f = 'assert';
  call_user_func_array($f, [false, 'assertion failed']);
}

function test_assert_map_pass() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);
  $f = 'assert';
  array_map($f, ['($var1 = \'300\') && true', '($var2 = \'400\') && true']);
  var_dump($var1, $var2);
}

function test_assert_map_pass2() {
  $f = 'assert';
  array_map($f, [true, true]);
}

function test_assert_map_fail() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);
  $f = 'assert';
  array_map($f, ['($var1 = \'300\') && false', '($var2 = \'400\') && false']);
  var_dump($var1, $var2);
}

function test_assert_map_fail2() {
  $f = 'assert';
  array_map($f, [false, false]);
}

function test_assert_walk_pass() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);
  $f = 'assert';
  $args = ['($var1 = \'300\') && true', '($var2 = \'400\') && true'];
  array_walk($args, $f);
  var_dump($var1, $var2);
}

function test_assert_walk_pass2() {
  $f = 'assert';
  $args = [true, true];
  array_walk($args, $f);
}

function test_assert_walk_fail() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);
  $f = 'assert';
  $args = ['($var1 = \'300\') && false', '($var2 = \'400\') && false'];
  array_walk($args, $f);
  var_dump($var1, $var2);
}

function test_assert_walk_fail2() {
  $f = 'assert';
  $args = [true, true];
  array_walk($args, $f);
}

function test_assert_chained() {
  $var1 = 100;
  $var2 = 200;
  var_dump($var1, $var2);
  $f = 'assert';
  call_user_func($f, 'extract([\'var1\' => \'300\', \'var2\' => \'400\']) || true');
  var_dump($var1, $var2);
}

function test_assert_errors() {
   $f = 'assert';
   $f();
   call_user_func($f);
   call_user_func_array($f, []);
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
  $current = array_shift($tests);
  apc_store('tests', $tests);
  return $current;
}

function run_tests() {
  $all = [
    'test_compact_dyncall',
    'test_compact_cuf',
    'test_compact_cuf_array',
    'test_compact_map',
    'test_compact_walk',
    'test_compact_errors',

    'test_extract_dyncall',
    'test_extract_cuf',
    'test_extract_cuf_array',
    'test_extract_map',
    'test_extract_walk',
    'test_extract_errors',

    'test_parse_str_dyncall',
    'test_parse_str_cuf',
    'test_parse_str_cuf_array',
    'test_parse_str_map',
    'test_parse_str_walk',
    'test_parse_str_errors',

    'test_framemeta_dyncall',
    'test_framemeta_cuf',
    'test_framemeta_cuf_array',
    'test_framemeta_map',
    'test_framemeta_errors',

    'test_getargs_dyncall',
    'test_getargs_cuf',
    'test_getargs_cuf_array',
    'test_getargs_errors',

    'test_assert_dyncall_pass',
    'test_assert_dyncall_pass2',
    'test_assert_dyncall_fail',
    'test_assert_dyncall_fail2',
    'test_assert_cuf_pass',
    'test_assert_cuf_pass2',
    'test_assert_cuf_fail',
    'test_assert_cuf_fail2',
    'test_assert_cuf_array_pass',
    'test_assert_cuf_array_pass2',
    'test_assert_cuf_array_fail',
    'test_assert_cuf_array_fail2',
    'test_assert_map_pass',
    'test_assert_map_pass2',
    'test_assert_map_fail',
    'test_assert_map_fail2',
    'test_assert_walk_pass',
    'test_assert_walk_pass2',
    'test_assert_walk_fail',
    'test_assert_walk_fail2',
    'test_assert_chained',
    'test_assert_errors',
  ];

  $test = get_current_test($all);
  if (is_null($test)) return;
  $func = $test[0];
  $iter = $test[1];
  echo "Running $func() #$iter....\n";
  $func();
  echo "$func() done\n";
}
