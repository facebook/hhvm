<?hh

require_once 'call_user_func.inc';

function test($f) {
  try {
    \var_dump(fb_call_user_func_async(__DIR__.'/call_user_func.inc', $f, 2));
  } catch (Exception $e) {
    print($e->getMessage()."\n");
  }
}

<<__EntryPoint>>
function main() {
  test('afunc');
  test(HH\fun('afunc'));
  test(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec['C', 'cfunc']));
  test(HH\class_meth('C', 'cfunc'));
  \var_dump(is_callable(HH\class_meth('C', 'cfunc')));
  \var_dump(call_user_func_array(HH\class_meth('C', 'cfunc'), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[2])));
  register_postsend_function(HH\class_meth('C', 'postSend'));
  register_shutdown_function(HH\class_meth('C', 'onShutdown'));
}
