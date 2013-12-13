<?hh

$obj = new stdclass();
$x0 = Vector {
1, $obj, "foo", $obj}
;
apc_store('x0', $x0);
$x1 = apc_fetch('x0');
var_dump($x1);
echo "========\n";
$x0 = Map {
'a' => 1, 'b' => $obj, 33 => "foo", 44 => $obj}
;
apc_store('x0', $x0);
$x1 = apc_fetch('x0');
var_dump($x1);
echo "========\n";
$x0 = StableMap {
'a' => 1, 'b' => $obj, 33 => "foo", 44 => $obj}
;
apc_store('x0', $x0);
$x1 = apc_fetch('x0');
var_dump($x1);
echo "========\n";
$x0 = Set {
1, "foo"}
;
apc_store('x0', $x0);
$x1 = apc_fetch('x0');
var_dump($x1);

