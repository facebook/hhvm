<?php
Class c {
	private $a;
	static private $b;
}

class d extends c {}

ReflectionClass::export("c");
ReflectionClass::export("d");
?>
