<?hh


<<__EntryPoint>>
function main_502() :mixed{
$a = varray[1, 2, 3];
$b = $a;
$b[] = 4;
var_dump($a);
var_dump($b);
$b = 3;
var_dump($a);
var_dump($b);
}
