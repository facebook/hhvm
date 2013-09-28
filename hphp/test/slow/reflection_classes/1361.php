<?php

interface A {
function foo();
}
interface B extends A {
}
class C implements B {
function foo() {
}
}
;
$klass = new ReflectionClass('C');
var_dump($klass->implementsInterface('A'));
$inter = new ReflectionClass('B');
var_dump($inter->hasMethod('foo'));
