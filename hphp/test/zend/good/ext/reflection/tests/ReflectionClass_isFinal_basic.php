<?php

class TestClass {}
final class TestFinalClass {}
<<__EntryPoint>> function main() {
$normalClass = new ReflectionClass('TestClass');
$finalClass = new ReflectionClass('TestFinalClass');

var_dump($normalClass->isFinal());
var_dump($finalClass->isFinal());
}
