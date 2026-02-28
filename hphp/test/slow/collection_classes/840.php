<?hh



<<__EntryPoint>>
function main_840() :mixed{
$m = Map {
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
$x1 = __hhvm_intrinsics\apc_fetch_no_check('x0');
$x1[3]['e'] = 5;
var_dump($x1);
}
