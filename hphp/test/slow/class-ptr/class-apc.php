<?hh

class main {}

<<__EntryPoint>>
function main() {
  apc_store('mainf', __hhvm_intrinsics\create_class_pointer('main'));
  apc_store('maina', array(1, __hhvm_intrinsics\create_class_pointer('main'), 'foo'));

  var_dump(__hhvm_intrinsics\apc_fetch_no_check('mainf'));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check('maina'));
}
