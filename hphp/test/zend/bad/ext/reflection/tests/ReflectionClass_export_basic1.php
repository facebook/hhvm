<?php
Class A { 
	public function privf(Exception $a) {}
	public function pubf(A $a,
						 $b,
						 C $c = null,
						 $d = K,
						 $e = "15 chars long -",
						 $f = null,
						 $g = false,
						 array $h = null) {}
}

Class C extends A { }

define('K', "16 chars long --");
ReflectionClass::export("C");
?>
