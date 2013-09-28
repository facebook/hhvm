<?php

interface A { 
	function a();
	function b();
}

class B implements A {
	public function a() { }
	public function b() { }
	
	public function __construct() {
		var_dump(get_class_methods('A'));
		var_dump(get_class_methods('B'));
	}
	
	public function __destruct() { }
}

new B;

?>