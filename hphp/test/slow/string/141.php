<?hh


<<__EntryPoint>>
function main_141() {
$a = 'test';
 $b = $a;
 $a[10] = 'ABC';
 var_dump($a);
var_dump($b);
}
