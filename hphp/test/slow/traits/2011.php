<?php

trait foo {
	
	public function __construct() {
		var_dump(__FUNCTION__);
	}
	public function __destruct() {
		var_dump(__FUNCTION__);
	}
}

class bar {
	use foo;
}

new bar;

?>
