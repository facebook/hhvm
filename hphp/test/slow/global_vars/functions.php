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

function test_get() {
  var_dump(HH\global_get(__FUNCTION__) is null);
  $GLOBALS[__FUNCTION__] = __FILE__;
  var_dump(HH\global_get(__FUNCTION__) === __FILE__);
}

function test_set() {
  var_dump($GLOBALS[__FUNCTION__] is null);
  HH\global_set(__FUNCTION__, __FILE__);
  var_dump($GLOBALS[__FUNCTION__] === __FILE__);
}

function test_bogus_args() {
  var_dump(HH\global_get(42));
}

function test_unset() {
  $GLOBALS[__FUNCTION__] = __FILE__;
  var_dump($GLOBALS[__FUNCTION__] === __FILE__);
  HH\global_unset(__FUNCTION__);
  var_dump($GLOBALS[__FUNCTION__] is null);
}

function test_global_keys() {
  $GLOBALS[__FUNCTION__] = null;
  var_dump(HH\global_keys() == keyset(array_keys($GLOBALS['GLOBALS'])));
  var_dump(array_key_exists(__FUNCTION__, HH\global_keys()));
}

function test_global_key_exists() {
  var_dump(HH\global_key_exists(__FUNCTION__));
  $GLOBALS[__FUNCTION__] = null;
  var_dump(array_key_exists(__FUNCTION__, $GLOBALS['GLOBALS']));
  var_dump(HH\global_key_exists(__FUNCTION__));
}

<<__EntryPoint>>
function main() {
  test_normal();
  test_functions();
  test_functions_arg(mt_rand(0,1) ? "derp" : "foo");
  test_get();
  test_set();
  test_bogus_args();
  test_unset();
  test_global_keys();
  test_global_key_exists();
}
