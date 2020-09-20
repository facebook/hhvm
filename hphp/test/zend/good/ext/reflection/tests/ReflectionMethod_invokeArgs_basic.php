<?hh

class TestClass {
    public $prop = 2;

    public function foo() {
        echo "Called foo(), property = $this->prop\n";
        var_dump($this);
        return "Return Val";
    }

    public function willThrow() {
        throw new Exception("Called willThrow()");
    }

    public function methodWithArgs($a, $b) {
        echo "Called methodWithArgs($a, $b)\n";
    }
}

<<__EntryPoint>> function main(): void {
$testClassInstance = new TestClass();
$testClassInstance->prop = "Hello";

$foo = new ReflectionMethod($testClassInstance, 'foo');
$methodWithArgs = new ReflectionMethod('TestClass', 'methodWithArgs');
$methodThatThrows = new ReflectionMethod("TestClass::willThrow");


echo "Public method:\n";

var_dump($foo->invokeArgs($testClassInstance, varray[]));
var_dump($foo->invokeArgs($testClassInstance, varray[true]));

echo "\nMethod with args:\n";

var_dump($methodWithArgs->invokeArgs($testClassInstance, varray[1, "arg2"]));
var_dump($methodWithArgs->invokeArgs($testClassInstance, varray[1, "arg2", 3]));

echo "\nMethod that throws an exception:\n";
try {
    $methodThatThrows->invokeArgs($testClassInstance, varray[]);
} catch (Exception $e) {
    var_dump($e->getMessage());
}
}
