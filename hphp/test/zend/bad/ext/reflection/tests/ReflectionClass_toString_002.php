<?php
Class A {
	function f() {}
}
Class B extends A {
	function f() {}
}
Class C extends B {

}
Class D extends C {
	function f() {}
}
foreach (array('A', 'B', 'C', 'D') as $class) {
	echo "\n\n----( Reflection class $class: )----\n";
	$rc = new ReflectionClass($class);
	echo $rc;
}

?>
