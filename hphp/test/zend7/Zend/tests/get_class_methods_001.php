<?php

abstract class X { 
	public function a() { }
	private function b() { }
	protected function c() { }	
}

class Y extends X {
	private function bb() { }
	
	static public function test() { 
		var_dump(get_class_methods('X'));
		var_dump(get_class_methods('Y'));
	}
}


var_dump(get_class_methods('X'));
var_dump(get_class_methods('Y'));


Y::test();

?>
