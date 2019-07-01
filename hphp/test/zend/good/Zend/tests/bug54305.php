<?hh
class TestClass {
    public function methodWithArgs($a, $b) {
    }
}
abstract class AbstractClass {
}
<<__EntryPoint>> function main(): void {
$methodWithArgs = new ReflectionMethod('TestClass', 'methodWithArgs');
echo $methodWithArgs++;
}
