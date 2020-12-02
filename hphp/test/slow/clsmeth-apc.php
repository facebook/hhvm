<?hh

class C { static function f() {} }

<<__EntryPoint>>
function main() {
  apc_store('mainf', class_meth(C::class, 'f'));
  apc_store('maina', varray[1, class_meth(C::class, 'f'), 'foo']);

  apc_store('sysf', class_meth(Map::class, 'fromItems'));
  apc_store('sysa', varray[10, class_meth(Map::class, 'fromItems'), 'foo']);

  var_dump(__hhvm_intrinsics\apc_fetch_no_check('mainf'));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check('maina'));

  var_dump(__hhvm_intrinsics\apc_fetch_no_check('sysf'));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check('sysa'));
}

