<?hh


<<__EntryPoint>>
function main_143() :mixed{
$a = 'test';
 $b = $a;
 $b[10] = 'ABC';
 var_dump($a);
var_dump($b);
}
