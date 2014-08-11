<?php

Class A {
	static private $a 	= 0;
	static protected $b = 1;
	static public $c 	= 2;
	
	private function f() {}
	private static function sf() {}
}

Class C extends A { }

ReflectionClass::export("C");

?>
