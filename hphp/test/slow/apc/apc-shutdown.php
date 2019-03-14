<?hh

function foo() {

  apc_store('x', array(Vector {1, 2, 3}));
  ApcApcShutdown::$z = apc_fetch('x');
  apc_store('x', null);
}


<<__EntryPoint>>
function main_apc_shutdown() {
foo();
echo "ok\n";
}

abstract final class ApcApcShutdown {
  public static $z;
}
