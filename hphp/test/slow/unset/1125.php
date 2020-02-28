<?hh


<<__EntryPoint>>
function main_1125() {
$a = varray[1,2];
unset($a[true]);
var_dump($a);
}
