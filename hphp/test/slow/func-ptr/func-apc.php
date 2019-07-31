<?hh

<<__EntryPoint>>
function main() {
  apc_store('mainf', fun('main'));
  apc_store('maina', array(1, fun('main'), 'foo'));

  var_dump(__hhvm_intrinsics\apc_fetch_no_check('mainf'));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check('maina'));
}
