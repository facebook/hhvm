<?php

interface TestInterface {}
class TestClass {}
interface DerivedInterface extends TestInterface {}

$reflectionClass = new ReflectionClass('TestInterface');
$reflectionClass2 = new ReflectionClass('TestClass');
$reflectionClass3 = new ReflectionClass('DerivedInterface');

var_dump($reflectionClass->isInterface());
var_dump($reflectionClass2->isInterface());
var_dump($reflectionClass3->isInterface());

?>
