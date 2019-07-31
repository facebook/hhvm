<?hh

function err($x, $y) { echo $y; echo "\n"; }
set_error_handler('err');
class Asd {
  private function __construct() {}
}

function get_name() {
  apc_store('name', 'Asd'); // make invisible to static analysis
  return __hhvm_intrinsics\apc_fetch_no_check('name');
}

function x() { $name = get_name(); new $name(); } x();
