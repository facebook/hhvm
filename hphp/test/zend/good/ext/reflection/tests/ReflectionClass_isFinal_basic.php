<?php

class TestClass {}
final class TestFinalClass {}

$normalClass = new ReflectionClass('TestClass');
$finalClass = new ReflectionClass('TestFinalClass');

var_dump($normalClass->isFinal());
var_dump($finalClass->isFinal());

?>
