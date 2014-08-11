<?php
Class A {
	private function f() {}
}
Class B extends A {
	private function f() {}
}
Class C extends B {

}
Class D extends C {
	private function f() {}
}
foreach (array('A', 'B', 'C', 'D') as $class) {
	echo "\n\n----( Reflection class $class: )----\n";
	$rc = new ReflectionClass($class);
	echo $rc;
}

?>
