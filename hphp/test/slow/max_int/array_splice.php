<?hh

<<__EntryPoint>>
function main_array_splice() {
$a = array(1, 2, 3);
var_dump(array_splice(inout $a, 0, PHP_INT_MAX));
}
