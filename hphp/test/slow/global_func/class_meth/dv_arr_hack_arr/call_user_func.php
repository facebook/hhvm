<?hh

function test($f) :mixed{
  try {
    \var_dump(fb_call_user_func_async(__DIR__.'/call_user_func.inc', $f, 2));
  } catch (Exception $e) {
    print($e->getMessage()."\n");
  }
}

<<__EntryPoint>>
function main() :mixed{
  require_once 'call_user_func.inc';

  test('afunc');
  test('afunc');
  test(darray(vec['C', 'cfunc']));
  \var_dump(is_callable(C::cfunc<>));
  \var_dump(call_user_func_array(C::cfunc<>, darray(vec[2])));
  register_postsend_function(C::postSend<>);
  register_shutdown_function(C::onShutdown<>);
}
