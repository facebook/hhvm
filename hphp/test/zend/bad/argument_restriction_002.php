<?php
Abstract Class Base {
	public function test($foo, array &$bar, $option = NULL, $extra = 3.141592653589793238462643383279502884197169399375105  ) {
	}	
}

class Sub extends Base {
	public function test($foo, array &$bar) {
	}	
}
?>