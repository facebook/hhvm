<?php

$test_array = array(
	0xcc5c4600 => 1,
	0xce331a00 => 2
);
$test_array[0xce359000] = 3;
  
var_dump($test_array);
var_dump($test_array[0xce331a00]);

class foo {
	public $x;
	
	public function __construct() {
		$this->x[0xce359000] = 3;
		var_dump($this->x);
	}
}

new foo;

?>