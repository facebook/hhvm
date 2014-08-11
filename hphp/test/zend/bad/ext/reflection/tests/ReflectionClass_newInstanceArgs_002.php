<?php
class A {
	public function __construct($a, $b) {
		echo "In constructor of class B with arg $a\n";
	}
}
$rc = new ReflectionClass('A');
$a = $rc->newInstanceArgs('x');
var_dump($a);

?>
