<?hh

<<__EntryPoint>>
function main_array_fill() :mixed{
$a = array_fill(5, 6, "banana");
$b = array_fill(-2, 2, "pear");
$c = array_fill(0, 0, "apple");
$d = array_fill(10, 0, "orange");
var_dump($a);
var_dump($b);
var_dump($c);
var_dump($d);
}
