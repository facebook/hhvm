<?php
class C {
}

interface iface {
	function f1();
}

class ifaceImpl implements iface {
	function f1() {}
}

abstract class abstractClass {
	function f1() {}
	abstract function f2();
}

class D extends abstractClass {
	function f2() {}
}

$classes = array("C", "iface", "ifaceImpl", "abstractClass", "D");

foreach($classes  as $class ) {
	$reflectionClass = new ReflectionClass($class);
	echo "Is $class instantiable?  ";
	var_dump($reflectionClass->IsInstantiable()); 

}

?>
