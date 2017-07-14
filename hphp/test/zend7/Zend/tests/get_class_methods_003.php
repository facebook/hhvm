<?php

interface I { 
	function aa();
	function bb();
	static function cc();
}

class X {
	public function a() { }
	protected function b() { }
	private function c() { }
	
	static public function static_a() { }
	static protected function static_b() { }
	static private function static_c() { }
}

class Y extends X implements I {
	public function aa() { }
	public function bb() { }
	
	static function cc() { }
	
	public function __construct() {
		var_dump(get_class_methods('I'));
		var_dump(get_class_methods('Y'));
		var_dump(get_class_methods('X'));
	}
	
	public function __destruct() { }
}

new Y;

?>
