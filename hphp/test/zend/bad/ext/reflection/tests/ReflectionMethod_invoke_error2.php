<?php

class TestClass {

    public function methodWithArgs($a, $b) {
        echo "Called methodWithArgs($a, $b)\n";
    }
}

$methodWithArgs = new ReflectionMethod('TestClass', 'methodWithArgs');

$testClassInstance = new TestClass();

echo "\nMethod with args:\n";
var_dump($methodWithArgs->invoke($testClassInstance));

?>
