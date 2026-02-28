<?hh

class bar {
 function baz() :AsyncGenerator<mixed,mixed,void>{
 yield 5;
 }
 }

<<__EntryPoint>>
function main_1341() :mixed{
$x = new ReflectionClass('bar');
var_dump(count($x->getMethods()));
}
