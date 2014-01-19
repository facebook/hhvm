<?hh

$m = StableMap {
'd' => 4}
;
$x0 = Vector {
Map {
'a' => 1}
, Map {
'b' => 2}
, Set {
'c'}
, $m, $m}
;
apc_store('x0', $x0);
$x1 = apc_fetch('x0');
$x1[3]['e'] = 5;
var_dump($x1);

