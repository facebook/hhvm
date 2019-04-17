<?hh // partial

function test_functions_arg($k) {
  HH\global_set($k, __FILE__);
  var_dump(HH\global_get($k) === __FILE__);
  HH\global_unset($k);
}

function test_functions() {
  HH\global_set(__FUNCTION__, __FILE__);
  var_dump(HH\global_get(__FUNCTION__) === __FILE__);
  HH\global_unset(__FUNCTION__);
}

function test_normal() {
  $GLOBALS[__FUNCTION__] = __FILE__;
  var_dump($GLOBALS[__FUNCTION__] === __FILE__);
  unset($GLOBALS[__FUNCTION__]);
}

function test_idx() {
  var_dump(HH\global_get_safe(__FUNCTION__) is null);
  $GLOBALS[__FUNCTION__] = __FILE__;
  var_dump(HH\global_get_safe(__FUNCTION__) === __FILE__);
}

function test_get() {
  $GLOBALS[__FUNCTION__] = __FILE__;
  var_dump(HH\global_get(__FUNCTION__) === __FILE__);
}

function test_set() {
  var_dump(idx($GLOBALS, __FUNCTION__) is null);
  HH\global_set(__FUNCTION__, __FILE__);
  var_dump($GLOBALS[__FUNCTION__] === __FILE__);
}

function test_bogus_args() {
  var_dump(HH\global_get(42));
}

function test_unset() {
  $GLOBALS[__FUNCTION__] = __FILE__;
  var_dump(idx($GLOBALS, __FUNCTION__) === __FILE__);
  HH\global_unset(__FUNCTION__);
  var_dump(idx($GLOBALS, __FUNCTION__) is null);
}

function test_get_safe() {
  var_dump((HH\global_get_safe(__FUNCTION__) ?? 'unset') === 'unset');
  $GLOBALS[__FUNCTION__] = __FILE__;
  var_dump((HH\global_get_safe(__FUNCTION__) ?? 'unset') === __FILE__);
}

<<__EntryPoint>>
function main() {
  test_normal();
  test_functions();
  test_functions_arg(mt_rand(0,1) ? "derp" : "foo");
  test_get();
  test_set();
  test_bogus_args();
  test_get_safe();
  test_unset();
}
