<?php
class A0 {}
class B0 extends A0 {}
abstract class A1 {}
class B1 extends A1 {}

interface I0 {}
interface I1 {}
interface I2 {}
interface I3 {}
interface I4 extends I3 {}
interface I5 extends I4 {}
interface I6 extends I5, I1, I2 {}
interface I7 extends I6 {}

class C0 implements I0 {}
class C1 implements I1, I3 {}
class C2 extends C1 {}
class C3 extends C2 implements I1 {}
class C4 extends C3 implements I2 {}
class C5 extends C4 implements I7 {}
class C6 implements I1, I2, I3, I4, I5, I6, I7 {}


$classes = array( 	'A0', 'A1', 'B0', 'B1', 
					'I0', 'I1', 'I2', 'I3', 'I4', 'I5', 'I6', 'I7',
					'C0', 'C1', 'C2', 'C3', 'C4', 'C5', 'C6'	);

foreach ($classes as $class) {
	echo "---( Interfaces implemented by $class )---\n ";
	$rc = new ReflectionClass($class);
	$interfaces = $rc->getInterfaces();
	// Sort interfaces so that tests do not fail because of wrong order.
	ksort($interfaces);
	print_r($interfaces);
}

?>
