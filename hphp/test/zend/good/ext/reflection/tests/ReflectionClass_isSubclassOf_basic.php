<?php
class A {}
class B extends A {}
class C extends B {}

interface I {}
class X implements I {}

$classNames = array('A', 'B', 'C', 'I', 'X'); 

foreach ($classNames as $className) {
	$rcs[$className] = new ReflectionClass($className);
}

foreach ($rcs as $childName => $child) {
	foreach ($rcs as $parentName => $parent) {
		echo "Is " . $childName . " a subclass of " . $parentName . "? \n";
		echo "   - Using object argument: ";
		var_dump($child->isSubclassOf($parent));
		echo "   - Using string argument: ";
		var_dump($child->isSubclassOf($parentName)); 
	}
}
?>
