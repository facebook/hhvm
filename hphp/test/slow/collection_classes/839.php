<?hh



<<__EntryPoint>>
function main_839() {
$obj = new stdClass();
$x0 = Vector {
1, $obj, "foo", $obj}
;
apc_store('x0', $x0);
$x1 = __hhvm_intrinsics\apc_fetch_no_check('x0');
var_dump($x1);
echo "========\n";
$x0 = Map {
'a' => 1, 'b' => $obj, 33 => "foo", 44 => $obj}
;
apc_store('x0', $x0);
$x1 = __hhvm_intrinsics\apc_fetch_no_check('x0');
var_dump($x1);
echo "========\n";
$x0 = Set {
1, "foo"}
;
apc_store('x0', $x0);
$x1 = __hhvm_intrinsics\apc_fetch_no_check('x0');
var_dump($x1);
}
