<?hh


<<__EntryPoint>>
function main_1124() {
$a = varray[1,2];
unset($a[false]);
var_dump($a);
}
