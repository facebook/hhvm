<?hh

interface A {
function foo():mixed;
}
interface B extends A {
}
class C implements B {
function foo() :mixed{
}
}

<<__EntryPoint>>
function main_1361() :mixed{
;
$klass = new ReflectionClass('C');
var_dump($klass->implementsInterface('A'));
$inter = new ReflectionClass('B');
var_dump($inter->hasMethod('foo'));
}
