<?hh


<<__EntryPoint>>
function main_504() :mixed{
$a = vec[1.5, 2.5, 3.5];
$b = $a;
$b[] = 4.5;
var_dump($a);
var_dump($b);
$b = 3.5;
var_dump($a);
var_dump($b);
}
