<?php

class A {
	protected $a = 1;
	private $b = 2;
}

class B extends A {
	private $c = 3;
	public function __construct() {
		var_dump(get_class_vars('A'));
		var_dump(get_class_vars('B'));
	}	
}

var_dump(get_class_vars('A'));
var_dump(get_class_vars('B'));

new B;

?>