<?hh


<<__EntryPoint>>
function main_506() :mixed{
$a = vec[1, 'hello', 3.5];
$b = $a;
$b[] = 'world';
var_dump($a);
var_dump($b);
$b = 3;
var_dump($a);
var_dump($b);
}
