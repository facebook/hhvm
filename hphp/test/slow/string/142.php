<?hh


<<__EntryPoint>>
function main_142() {
$a = 'test';
 $b = $a;
 $b[0] = 'ABC';
 var_dump($a);
 var_dump($b);
}
