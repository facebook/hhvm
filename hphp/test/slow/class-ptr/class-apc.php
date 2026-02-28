<?hh

<<__DynamicallyReferenced>> class main {}

<<__EntryPoint>>
function main() :mixed{
  apc_store('mainf', HH\classname_to_class('main'));
  apc_store('maina', vec[1, HH\classname_to_class('main'), 'foo']);

  var_dump(__hhvm_intrinsics\apc_fetch_no_check('mainf'));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check('maina'));
}
