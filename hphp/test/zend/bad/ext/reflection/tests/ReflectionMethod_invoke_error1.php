<?php

class TestClass {
    public $prop = 2;

    public function foo() {
        echo "Called foo(), property = $this->prop\n";
        var_dump($this);
        return "Return Val";
    }

    private static function privateMethod() {
        echo "Called privateMethod()\n";
    }
}

abstract class AbstractClass {
    abstract function foo();
}

$foo = new ReflectionMethod('TestClass', 'foo');
$privateMethod = new ReflectionMethod("TestClass::privateMethod");

$testClassInstance = new TestClass();
$testClassInstance->prop = "Hello";

echo "invoke() on a non-object:\n";
try {
    var_dump($foo->invoke(true));
} catch (ReflectionException $e) {
    var_dump($e->getMessage());
}

echo "\ninvoke() on a non-instance:\n";
try {
    var_dump($foo->invoke(new stdClass()));
} catch (ReflectionException $e) {
    var_dump($e->getMessage());
}

echo "\nPrivate method:\n";
try {
    var_dump($privateMethod->invoke($testClassInstance));
} catch (ReflectionException $e) {
    var_dump($e->getMessage());
}

echo "\nAbstract method:\n";
$abstractMethod = new ReflectionMethod("AbstractClass::foo");
try {
    $abstractMethod->invoke(true);
} catch (ReflectionException $e) {
    var_dump($e->getMessage());
}

?>
