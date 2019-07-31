<?hh


<<__EntryPoint>>
function main_apc_store_as_primed_do_not_use() {
var_dump(apc_store_as_primed_do_not_use('foo', 'bar'));
var_dump(__hhvm_intrinsics\apc_fetch_no_check('foo'));
}
