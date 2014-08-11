<?php
class A {}
class B extends A {}

$rc = new ReflectionClass('B');
$parent = $rc->getParentClass();
$grandParent = $parent->getParentClass();
var_dump($parent, $grandParent);

echo "\nTest bad params:\n";
var_dump($rc->getParentClass(null));
var_dump($rc->getParentClass('x'));
var_dump($rc->getParentClass('x', 123));

?>
