<?php

function reflectMethod($class, $method) {
    $methodInfo = new ReflectionMethod($class, $method);
    echo "**********************************\n";
    echo "Reflecting on method $class::$method()\n\n";
    echo "\ngetName():\n";
    var_dump($methodInfo->getName());
    echo "\nisInternal():\n";
    var_dump($methodInfo->isInternal());
    echo "\nisUserDefined():\n";
    var_dump($methodInfo->isUserDefined());
    echo "\n**********************************\n";
}

class TestClass
{
    public function foo() {
        echo "Called foo()\n";
    }

    static function stat() {
        echo "Called stat()\n";
    }

    private function priv() {
        echo "Called priv()\n";
    }

    protected function prot() {}

    public function __destruct() {}
}

class DerivedClass extends TestClass {}

interface TestInterface {
    public function int();
}

reflectMethod("DerivedClass", "foo");
reflectMethod("TestClass", "stat");
reflectMethod("TestClass", "priv");
reflectMethod("TestClass", "prot");
reflectMethod("DerivedClass", "prot");
reflectMethod("TestInterface", "int");
reflectMethod("ReflectionProperty", "__construct");
reflectMethod("TestClass", "__destruct");


?>
