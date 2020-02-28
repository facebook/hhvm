<?hh


<<__EntryPoint>>
function main_409() {
$a = darray['a' => 'va'];
 $a += darray['c' => varray[3]];
 var_dump($a);
}
