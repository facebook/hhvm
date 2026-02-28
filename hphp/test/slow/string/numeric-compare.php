<?hh

<<__EntryPoint>>
function main_numeric_compare() :mixed{
$a = '123123123123123123123';
$b = '123123123123123123124';
$c = '1231231231231231231234';
var_dump($a == $b);
var_dump($a == $c);
var_dump((int) $a === (int) $b);
var_dump((int) $a === (int) $c);
}
