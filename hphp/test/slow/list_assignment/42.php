<?hh


<<__EntryPoint>>
function main_42() {
$info = varray['coffee', 'brown', 'caffeine'];
$a = darray[];
list($a[0], $a[1], $a[2]) = $info;
var_dump($a);
}
