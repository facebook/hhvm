<?php
class Foo {
	const c1 = 1;
}
$class = new ReflectionClass("Foo");
var_dump($class->getConstant("c1"));
var_dump($class->getConstant("c2"));
?>
