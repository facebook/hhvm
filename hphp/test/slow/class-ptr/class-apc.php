<?hh

class main {}

<<__EntryPoint>>
function main() {
  apc_store('mainf', __hhvm_intrinsics\create_class_pointer('main'));
  apc_store('maina', array(1, __hhvm_intrinsics\create_class_pointer('main'), 'foo'));

  var_dump(apc_fetch('mainf'));
  var_dump(apc_fetch('maina'));
}
