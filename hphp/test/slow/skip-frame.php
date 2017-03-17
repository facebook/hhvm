<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function make_wrapper($action, $layers=3) {
  if ($layers > 0) {
    $c = make_wrapper($action, $layers-1);
    return ['call_user_func_array', ['array_map'], [[$c[0], $c[1], $c[2]]]];
  } else {
    return ['call_user_func_array', ['assert'], [["($action) || true"]]];
  }
}

function change_local($x = 123) {
  $var = [$x, $x, $x];
  $wrapper = make_wrapper('$var = 1234');
  array_map($wrapper[0], $wrapper[1], $wrapper[2]);
  var_dump($var + $var);
}

function add_local() {
  $wrapper = make_wrapper('$var = "abcd"');
  array_map($wrapper[0], $wrapper[1], $wrapper[2]);
  var_dump(get_defined_vars()['var']);
}

function all_vars() {
  $var1 = 123;
  $var2 = 456;
  $var3 = 789;
  $wrapper = make_wrapper('var_dump(get_defined_vars()["var2"])');
  array_map($wrapper[0], $wrapper[1], $wrapper[2]);
}

function compact_extract() {
  $wrapper = make_wrapper('extract(["var1" => 123, "var2" => 456, "var3" => 789])');
  array_map($wrapper[0], $wrapper[1], $wrapper[2]);
  $wrapper = make_wrapper('var_dump(compact("var1", "var2", "var3"))');
  array_map($wrapper[0], $wrapper[1], $wrapper[2]);
}

function change_local_method($x = 123) {
  $var = [$x, $x, $x];
  $v = Vector{'($var = 1234) || true'};
  $v->map('assert');
  var_dump($var + $var);
}

function all_vars_method() {
  $var1 = 123;
  $var2 = 456;
  $var3 = 789;
  $v = Vector{'(var_dump(get_defined_vars()["var2"])) || true'};
  $v->map('assert');
}

function get_arg_impl($arg1, $arg2, $arg3) {
  var_dump(array_map('func_get_arg', [0]));
}
function get_arg() { get_arg_impl(1, 2, 3); }

function run($c) {
  $c();
  $c();
}

run('change_local');
run('add_local');
run('all_vars');
run('compact_extract');
run('get_arg');
run('change_local_method');
run('all_vars_method');
