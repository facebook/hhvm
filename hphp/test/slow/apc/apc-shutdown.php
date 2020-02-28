<?hh

function foo() {

  apc_store('x', varray[Vector {1, 2, 3}]);
  ApcApcShutdown::$z = __hhvm_intrinsics\apc_fetch_no_check('x');
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
