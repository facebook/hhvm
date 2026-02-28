<?hh

function f($x) :mixed{
 $b = $x;
 $b++;
 }

<<__EntryPoint>>
function main_29() :mixed{
$a = 1;
f($a);
var_dump($a);
}
