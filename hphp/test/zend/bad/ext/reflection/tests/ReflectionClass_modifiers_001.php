<?php
abstract class A {}
class B extends A {}
class C {}
final class D {}
interface I {}

$classes = array("A", "B", "C", "D", "I");

foreach ($classes as $class) {
	$rc = new ReflectionClass($class);
	var_dump($rc->isFinal());
	var_dump($rc->isInterface());
	var_dump($rc->isAbstract());
	var_dump($rc->getModifiers());
}
?>
