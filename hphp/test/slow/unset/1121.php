<?hh

function rmv($a, $b) {
 unset($a[$b]);
 return $a;
 }

<<__EntryPoint>>
function main_1121() {
$a = varray['foo'];
$b = varray[];
var_dump(rmv($a, $b));
}
