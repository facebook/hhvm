<?php

abstract class A { 
	public function a() { }
	private function b() { }
	protected function c() { }	
}

class B extends A {
	private function bb() { }
	
	static public function test() { 
		var_dump(get_class_methods('A'));
		var_dump(get_class_methods('B'));
	}
}


var_dump(get_class_methods('A'));
var_dump(get_class_methods('B'));


B::test();

?>