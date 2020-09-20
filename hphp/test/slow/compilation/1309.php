<?hh

function foo() {
 return varray[1,2,3];
 }
function bar($a, $b) {
 $a = 4;
 }

<<__EntryPoint>>
function main_1309() {
$x = foo();
try { bar($x[3][4], $y); } catch (Exception $e) { echo $e->getMessage()."\n"; }
var_dump($x);
}
