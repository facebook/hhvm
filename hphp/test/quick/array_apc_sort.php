<?hh <<__EntryPoint>> function main(): void {
apc_store('foo', array("foo"));
$a = __hhvm_intrinsics\apc_fetch_no_check('foo');
var_dump($a);
sort(&$a);
var_dump($a);
}
