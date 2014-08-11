<?php
class privateCtorOld {
	private function privateCtorOld() {}
}
$reflectionClass = new ReflectionClass("privateCtorOld");

var_dump($reflectionClass->IsInstantiable('X'));
var_dump($reflectionClass->IsInstantiable(0, null));

?>
