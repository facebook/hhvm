<?hh


<<__EntryPoint>>
function main_1126() {
$a = darray[0 => 1, 1 => 2, '' => 'foo'];
unset($a[null]);
var_dump($a);
}
