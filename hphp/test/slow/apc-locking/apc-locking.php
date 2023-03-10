<?hh

function thread_main() {
  echo "In thread_main\n";
  $res = false;
  var_dump(apc_cas  ('whoops', 1, 2));
  var_dump(apc_inc  ('whoops', 1, inout $res), $res);
  var_dump(apc_fetch('whoops', inout $res), $res);
  var_dump(apc_fetch('whoops', inout $res), $res);
}

<<__EntryPoint>>
function main() {
  if (HH\execution_context() === "xbox") return;
  echo "In main\n";

  require_once('apc-locking.inc'); // define Foo

  apc_store('whoops', new Foo);

  fb_call_user_func_async(
    __FILE__,
    'thread_main'
  );
}
