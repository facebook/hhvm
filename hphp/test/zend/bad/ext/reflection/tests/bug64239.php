<?php
class A {
	use T2 { t2method as Bmethod; }
}
trait T2 {
	public function t2method() {
	}
}

class B extends A{
}

$obj = new ReflectionClass("B");
print_r($obj->getMethods());
print_r(($method = $obj->getMethod("Bmethod")));
var_dump($method->getName());
var_dump($method->getShortName());
?>
