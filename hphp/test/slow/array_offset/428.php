<?hh


<<__EntryPoint>>
function main_428() {
$a = darray[];
$a[null] = 10;
var_dump($a[null]);
var_dump($a[""]);
var_dump($a['']);
}
