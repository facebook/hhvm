<?php
error_reporting(E_ALL);

class A {
  private $b = 'b';
}
class C {
  static private $d = 'd';
}

$prop1 = (new ReflectionClass('A'))->getProperty('b');
$prop2 = (new ReflectionClass('A'))->getProperty('b');
$prop1->setAccessible(true);
try {
	$prop2->getValue(new A);
} catch(ReflectionException $e) {
	echo $e->getMessage(), "\n";
}

$prop1 = (new ReflectionClass('C'))->getProperty('d');
$prop2 = (new ReflectionClass('C'))->getProperty('d');
$prop1->setAccessible(true);
try {
	$prop2->getValue('C');
} catch(ReflectionException $e) {
	echo $e->getMessage(), "\n";
}
