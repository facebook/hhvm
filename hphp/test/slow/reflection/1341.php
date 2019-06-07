<?hh

class bar {
 function baz() {
 yield 5;
 }
 }

<<__EntryPoint>>
function main_1341() {
$x = new ReflectionClass('bar');
var_dump(count($x->getMethods()));
}
