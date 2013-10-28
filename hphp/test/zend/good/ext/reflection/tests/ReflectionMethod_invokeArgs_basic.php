<?php

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


$testClassInstance = new TestClass();
$testClassInstance->prop = "Hello";

$foo = new ReflectionMethod($testClassInstance, 'foo');
$methodWithArgs = new ReflectionMethod('TestClass', 'methodWithArgs');
$methodThatThrows = new ReflectionMethod("TestClass::willThrow");


echo "Public method:\n";

var_dump($foo->invokeArgs($testClassInstance, array()));
var_dump($foo->invokeArgs($testClassInstance, array(true)));

echo "\nMethod with args:\n";

var_dump($methodWithArgs->invokeArgs($testClassInstance, array(1, "arg2")));
var_dump($methodWithArgs->invokeArgs($testClassInstance, array(1, "arg2", 3)));

echo "\nMethod that throws an exception:\n";
try {
    $methodThatThrows->invokeArgs($testClassInstance, array());
} catch (Exception $e) {
    var_dump($e->getMessage());
}

?>