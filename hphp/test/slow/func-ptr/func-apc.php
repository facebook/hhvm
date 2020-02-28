<?hh

<<__EntryPoint>>
function main() {
  apc_store('mainf', fun('main'));
  apc_store('maina', varray[1, fun('main'), 'foo']);

  apc_store('sysf', fun('array_map'));
  apc_store('sysa', varray[10, fun('apc_fetch'), 'foo']);

  var_dump(__hhvm_intrinsics\apc_fetch_no_check('mainf'));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check('maina'));

  var_dump(__hhvm_intrinsics\apc_fetch_no_check('sysf'));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check('sysa'));
}
