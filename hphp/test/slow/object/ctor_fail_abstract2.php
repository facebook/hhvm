<?hh

function err($x, $y) { echo $y; echo "\n"; }
abstract class Asd {
  private function __construct() {}
}

function get_name() {
  apc_store('name', 'Asd'); // make invisible to static analysis
  return __hhvm_intrinsics\apc_fetch_no_check('name');
}

function x() { $name = get_name(); new $name(); }

<<__EntryPoint>>
function entrypoint_ctor_fail_abstract2(): void {
  set_error_handler(fun('err'));
  x();
}
