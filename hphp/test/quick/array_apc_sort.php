<?hh <<__EntryPoint>> function main(): void {
apc_store('foo', varray["foo"]);
$a = __hhvm_intrinsics\apc_fetch_no_check('foo');
var_dump($a);
sort(inout $a);
var_dump($a);
}
