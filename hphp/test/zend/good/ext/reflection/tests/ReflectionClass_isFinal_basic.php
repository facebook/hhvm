<?hh

class TestClass {}
final class TestFinalClass {}
<<__EntryPoint>> function main(): void {
$normalClass = new ReflectionClass('TestClass');
$finalClass = new ReflectionClass('TestFinalClass');

var_dump($normalClass->isFinal());
var_dump($finalClass->isFinal());
}
