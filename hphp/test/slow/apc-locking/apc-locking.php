<?hh

class Foo {
  function __wakeup() {
    echo "In wakeup\n";

    var_dump(count(apc_cache_info()));
    var_dump(apc_delete('whoops'));
  }
}

function thread_main() :mixed{
  echo "In thread_main\n";
  $res = false;
  var_dump(apc_cas  ('whoops', 1, 2));
  var_dump(apc_inc  ('whoops', 1, inout $res), $res);
  var_dump(apc_fetch('whoops', inout $res), $res);
  var_dump(apc_fetch('whoops', inout $res), $res);
}

<<__EntryPoint>>
function main() :mixed{
  if (HH\execution_context() === "xbox") return;
  echo "In main\n";

  apc_store('whoops', new Foo);

  $r = fb_call_user_func_async(
    __FILE__,
    'thread_main'
  );
  fb_end_user_func_async($r);
}
