<?php
$r1 = new ReflectionClass("stdClass");

$myInstance = new stdClass;
$r2 = new ReflectionClass($myInstance);

class TrickClass {
	function __toString() {
		//Return the name of another class
		return "Exception";
	}
}
$myTrickClass = new TrickClass;
$r3 = new ReflectionClass($myTrickClass);

var_dump($r1, $r2, $r3);
?>
