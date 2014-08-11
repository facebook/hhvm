<?php
class A {
	public function A() {
		echo "In constructor of class A\n"; 
	}	
}

class B {
	public function __construct($a, $b) {
		echo "In constructor of class B with args $a, $b\n";
	}
}

class C {
	protected function __construct() {
		echo "In constructor of class C\n"; 
	}
}

class D {
	private function __construct() {
		echo "In constructor of class D\n";		
	}
}
class E {	
}


$rcA = new ReflectionClass('A');
$rcB = new ReflectionClass('B');
$rcC = new ReflectionClass('C');
$rcD = new ReflectionClass('D');
$rcE = new ReflectionClass('E');

$a1 = $rcA->newInstanceArgs();
$a2 = $rcA->newInstanceArgs(array('x'));
var_dump($a1, $a2);

$b1 = $rcB->newInstanceArgs();
$b2 = $rcB->newInstanceArgs(array('x', 123));
var_dump($b1, $b2);

try {
	$rcC->newInstanceArgs();
	echo "you should not see this\n";
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}

try {
	$rcD->newInstanceArgs();
	echo "you should not see this\n";
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}

$e1 = $rcE->newInstanceArgs();
var_dump($e1);

try {
	$e2 = $rcE->newInstanceArgs(array('x'));
	echo "you should not see this\n";
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
