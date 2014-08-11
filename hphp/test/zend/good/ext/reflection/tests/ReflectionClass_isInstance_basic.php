<?php
class A {}
class B extends A {}

interface I {}
class C implements I {}

class X {}

$classes = array("A", "B", "C", "I", "X");

$instances = array(	"myA" => new A,
					"myB" => new B,
					"myC" => new C,
					"myX" => new X );

foreach ($classes as $class) {
	$rc = new ReflectionClass($class);

	foreach ($instances as $name => $instance) {
		echo "is $name a $class? ";
		var_dump($rc->isInstance($instance));	
	}

}

?>
