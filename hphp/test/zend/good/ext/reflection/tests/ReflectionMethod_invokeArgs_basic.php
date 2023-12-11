<?hh

class TestClass {
    public $prop = 2;

    <<__DynamicallyCallable>> public function foo() :mixed{
        echo "Called foo(), property = $this->prop\n";
        var_dump($this);
        return "Return Val";
    }

    <<__DynamicallyCallable>> public function willThrow() :mixed{
        throw new Exception("Called willThrow()");
    }

    <<__DynamicallyCallable>> public function methodWithArgs($a, $b) :mixed{
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

var_dump($foo->invokeArgs($testClassInstance, vec[]));
var_dump($foo->invokeArgs($testClassInstance, vec[true]));

echo "\nMethod with args:\n";

var_dump($methodWithArgs->invokeArgs($testClassInstance, vec[1, "arg2"]));
var_dump($methodWithArgs->invokeArgs($testClassInstance, vec[1, "arg2", 3]));

echo "\nMethod that throws an exception:\n";
try {
    $methodThatThrows->invokeArgs($testClassInstance, vec[]);
} catch (Exception $e) {
    var_dump($e->getMessage());
}
}
