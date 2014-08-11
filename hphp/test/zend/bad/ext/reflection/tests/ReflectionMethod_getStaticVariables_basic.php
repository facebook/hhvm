<?php

class TestClass {
    public function foo() {
        static $c;
        static $a = 1;
        static $b = "hello";
        $d = 5;
    }

    private function bar() {
        static $a = 1;
    }

    public function noStatics() {
        $a = 54;
    }
}

echo "Public method:\n";
$methodInfo = new ReflectionMethod('TestClass::foo');
var_dump($methodInfo->getStaticVariables());

echo "\nPrivate method:\n";
$methodInfo = new ReflectionMethod('TestClass::bar');
var_dump($methodInfo->getStaticVariables());

echo "\nMethod with no static variables:\n";
$methodInfo = new ReflectionMethod('TestClass::noStatics');
var_dump($methodInfo->getStaticVariables());

echo "\nInternal Method:\n";
$methodInfo = new ReflectionMethod('ReflectionClass::getName');
var_dump($methodInfo->getStaticVariables());

?>
