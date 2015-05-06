<?php

class A {
    function foo() {}
}

class B extends A {
    function bar() {}
}

$methodInfo = new ReflectionMethod('B', 'foo');
var_dump($methodInfo->getDeclaringClass());

$methodInfo = new ReflectionMethod('B', 'bar');
var_dump($methodInfo->getDeclaringClass());

?> 
