<?hh


<<__EntryPoint>>
function main_1123() {
$a = varray[1,2];
unset($a[1.5]);
var_dump($a);
}
