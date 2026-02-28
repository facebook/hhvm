<?hh

function foo() :mixed{

  apc_store('x', vec[Vector {1, 2, 3}]);
  ApcApcShutdown::$z = __hhvm_intrinsics\apc_fetch_no_check('x');
  apc_store('x', null);
}


<<__EntryPoint>>
function main_apc_shutdown() :mixed{
foo();
echo "ok\n";
}

abstract final class ApcApcShutdown {
  public static $z;
}
