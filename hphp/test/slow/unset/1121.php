<?hh

function rmv($a, $b) {
 unset($a[$b]);
 return $a;
 }

<<__EntryPoint>>
function main_1121() {
$a = varray['foo'];
$b = array();
var_dump(rmv($a, $b));
}
