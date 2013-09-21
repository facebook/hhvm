<?php

interface A { 
	function aa();
	function bb();
	static function cc();
}

class C {
	public function a() { }
	protected function b() { }
	private function c() { }
	
	static public function static_a() { }
	static protected function static_b() { }
	static private function static_c() { }
}

class B extends C implements A {
	public function aa() { }
	public function bb() { }
	
	static function cc() { }
	
	public function __construct() {
		var_dump(get_class_methods('A'));
		var_dump(get_class_methods('B'));
		var_dump(get_class_methods('C'));
	}
	
	public function __destruct() { }
}

new B;

?>