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
  test(hh\fun('afunc'));
  test(array('C', 'cfunc'));
  test(hh\class_meth('C', 'cfunc'));
  \var_dump(is_callable(hh\class_meth('C', 'cfunc')));
  \var_dump(call_user_func_array(hh\class_meth('C', 'cfunc'), array(2)));
  register_postsend_function(hh\class_meth('C', 'postSend'));
  register_shutdown_function(hh\class_meth('C', 'onShutdown'));
}
