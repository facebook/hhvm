<?hh

function foo() :mixed{
 return varray[1,2,3];
 }
function bar($a, $b) :mixed{
 $a = 4;
 }

<<__EntryPoint>>
function main_1309() :mixed{
$x = foo();
try { bar($x[3][4], $y); } catch (Exception $e) { echo $e->getMessage()."\n"; }
var_dump($x);
}
