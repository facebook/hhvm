<?hh

function autoloader($type, $name, $err) {
  echo "In autoloader\n";

  var_dump(count(apc_cache_info()));
  var_dump(apc_delete('whoops'));
}

function thread_main() {
  echo "In thread_main\n";

  HH\autoload_set_paths(
    [
      'class' => [],
      'constant' => [],
      'function' => [],
      'type' => [],
      'failure' => 'autoloader'
    ],
    ''
  );
  $res = false;
  var_dump(apc_cas  ('whoops', 1, 2));
  var_dump(apc_inc  ('whoops', 1, &$res), $res);
  var_dump(apc_fetch('whoops', &$res), $res);
  var_dump(apc_fetch('whoops', &$res), $res);
}

<<__EntryPoint>>
function main() {
  echo "In main\n";

  require_once('apc-locking.inc'); // defnie Foo

  apc_store('whoops', new Foo);

  fb_call_user_func_async(
    __FILE__,
    'thread_main'
  );
}
