<?php

class A {
	static public $a, $aa;
	static private $b, $bb;
	static protected $c, $cc;

	static public function test() {
		var_dump(get_class_vars(__CLASS__));
	}
}

var_dump(get_class_vars('A'));
var_dump(A::test());

?>