<?php
Abstract Class Base {
	public function test($foo, $extra = array("test")) {
	}	
}

class Sub extends Base {
	public function test($foo, $extra) {
	}	
}
?>