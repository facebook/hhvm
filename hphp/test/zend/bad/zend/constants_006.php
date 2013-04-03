<?php

namespace test;

var_dump(__dir__);
var_dump(__file__);
var_dump(__line__);

class foo {
	public function __construct() {
		var_dump(__method__);
		var_dump(__class__);
		var_dump(__function__);
	}
}

new foo;

var_dump(__namespace__);

?>