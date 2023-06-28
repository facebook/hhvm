<?hh

<<__EntryPoint>>
function main() :mixed{
  apc_store('mainf', main<>);
  apc_store('maina', varray[1, main<>, 'foo']);

  apc_store('sysf', array_map<>);
  apc_store('sysa', varray[10, apc_fetch<>, 'foo']);

  var_dump(__hhvm_intrinsics\apc_fetch_no_check('mainf'));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check('maina'));

  var_dump(__hhvm_intrinsics\apc_fetch_no_check('sysf'));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check('sysa'));
}
