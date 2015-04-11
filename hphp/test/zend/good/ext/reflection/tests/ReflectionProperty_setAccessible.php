<?php
class A {
    protected $protected = 'a';
    protected static $protectedStatic = 'b';
    private $private = 'c';
    private static $privateStatic = 'd';
}

class B extends A {}

$a               = new A;
$protected       = new ReflectionProperty($a, 'protected');
$protectedStatic = new ReflectionProperty('A', 'protectedStatic');
$private         = new ReflectionProperty($a, 'private');
$privateStatic   = new ReflectionProperty('A', 'privateStatic');

try {
    var_dump($protected->getValue($a));
}

catch (ReflectionException $e) {
    var_dump($e->getMessage());
}

try {
    var_dump($protectedStatic->getValue());
}

catch (ReflectionException $e) {
    var_dump($e->getMessage());
}

try {
    var_dump($private->getValue($a));
}

catch (ReflectionException $e) {
    var_dump($e->getMessage());
}

try {
    var_dump($privateStatic->getValue());
}

catch (ReflectionException $e) {
    var_dump($e->getMessage());
}

$protected->setAccessible(TRUE);
$protectedStatic->setAccessible(TRUE);
$private->setAccessible(TRUE);
$privateStatic->setAccessible(TRUE);

var_dump($protected->getValue($a));
var_dump($protectedStatic->getValue());
var_dump($private->getValue($a));
var_dump($privateStatic->getValue());

$protected->setValue($a, 'e');
$protectedStatic->setValue('f');
$private->setValue($a, 'g');
$privateStatic->setValue('h');

var_dump($protected->getValue($a));
var_dump($protectedStatic->getValue());
var_dump($private->getValue($a));
var_dump($privateStatic->getValue());

$a               = new A;
$b               = new B;
$protected       = new ReflectionProperty($b, 'protected');
$protectedStatic = new ReflectionProperty('B', 'protectedStatic');
$private         = new ReflectionProperty($a, 'private');

try {
    var_dump($protected->getValue($b));
}

catch (ReflectionException $e) {
    var_dump($e->getMessage());
}

try {
    var_dump($protectedStatic->getValue());
}

catch (ReflectionException $e) {
    var_dump($e->getMessage());
}

try {
    var_dump($private->getValue($b));
}

catch (ReflectionException $e) {
    var_dump($e->getMessage());
}

$protected->setAccessible(TRUE);
$protectedStatic->setAccessible(TRUE);
$private->setAccessible(TRUE);

var_dump($protected->getValue($b));
var_dump($protectedStatic->getValue());
var_dump($private->getValue($b));

$protected->setValue($b, 'e');
$protectedStatic->setValue('f');
$private->setValue($b, 'g');

var_dump($protected->getValue($b));
var_dump($protectedStatic->getValue());
var_dump($private->getValue($b));
?>
