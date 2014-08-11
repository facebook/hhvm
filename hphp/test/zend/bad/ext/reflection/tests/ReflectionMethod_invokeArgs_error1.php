<?php

class TestClass {

    public function methodWithArgs($a, $b) {
        echo "Called methodWithArgs($a, $b)\n";
    }
}

abstract class AbstractClass {
    abstract function foo();
}

$methodWithArgs = new ReflectionMethod('TestClass', 'methodWithArgs');

$testClassInstance = new TestClass();

echo "\nMethod with args:\n";
var_dump($methodWithArgs->invokeArgs($testClassInstance, array())); 

?>
