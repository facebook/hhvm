<?hh


<<__EntryPoint>>
function main_140() {
$a = 'test';
 $b = $a;
 $a[0] = 'ABC';
 var_dump($a);
 var_dump($b);
}
