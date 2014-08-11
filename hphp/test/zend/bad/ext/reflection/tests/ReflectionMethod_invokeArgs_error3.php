<?php

class TestClass {
    public $prop = 2;

    public function foo() {
        echo "Called foo(), property = $this->prop\n";
        var_dump($this);
        return "Return Val";
    }

    public static function staticMethod() {
        echo "Called staticMethod()\n";
        var_dump($this);
    }

    private static function privateMethod() {
        echo "Called privateMethod()\n";
    }
}

abstract class AbstractClass {
    abstract function foo();
}

$testClassInstance = new TestClass();
$testClassInstance->prop = "Hello";

$foo = new ReflectionMethod($testClassInstance, 'foo');
$staticMethod = new ReflectionMethod('TestClass::staticMethod');
$privateMethod = new ReflectionMethod("TestClass::privateMethod");

echo "Wrong number of parameters:\n";
var_dump($foo->invokeArgs());
var_dump($foo->invokeArgs(true));

echo "\nNon-instance:\n";
try {
    var_dump($foo->invokeArgs(new stdClass(), array()));
} catch (ReflectionException $e) {
    var_dump($e->getMessage());
}

echo "\nNon-object:\n";
var_dump($foo->invokeArgs(true, array()));

echo "\nStatic method:\n";

var_dump($staticMethod->invokeArgs());
var_dump($staticMethod->invokeArgs(true));
var_dump($staticMethod->invokeArgs(true, array()));
var_dump($staticMethod->invokeArgs(null, array()));

echo "\nPrivate method:\n";
try {
    var_dump($privateMethod->invokeArgs($testClassInstance, array()));
} catch (ReflectionException $e) {
    var_dump($e->getMessage());
}

echo "\nAbstract method:\n";
$abstractMethod = new ReflectionMethod("AbstractClass::foo");
try {
    $abstractMethod->invokeArgs($testClassInstance, array());
} catch (ReflectionException $e) {
    var_dump($e->getMessage());
}
try {
    $abstractMethod->invokeArgs(true);
} catch (ReflectionException $e) {
    var_dump($e->getMessage());
}

?>
