<?hh

class C { static function f() :mixed{} }

<<__EntryPoint>>
function main() :mixed{
  apc_store('mainf', C::f<>);
  apc_store('maina', vec[1, C::f<>, 'foo']);

  apc_store('sysf', HH\Map::fromItems<>);
  apc_store('sysa', vec[10, HH\Map::fromItems<>, 'foo']);

  var_dump(__hhvm_intrinsics\apc_fetch_no_check('mainf'));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check('maina'));

  var_dump(__hhvm_intrinsics\apc_fetch_no_check('sysf'));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check('sysa'));
}

