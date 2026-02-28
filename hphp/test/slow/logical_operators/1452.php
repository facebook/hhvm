<?hh

function foo($a) :mixed{
 echo "foo";
 return $a;
 }

<<__EntryPoint>>
function main_1452() :mixed{
$x = true;
($x = $x) && foo(false);
var_dump($x);
$x = $x && foo(false);
var_dump($x);
$x = false;
($x = $x) || foo(true);
var_dump($x);
$x = $x || foo(true);
var_dump($x);
}
