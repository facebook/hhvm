<?php

function non_class_scope() {
	return true;
}

class test {
	protected $prop = 1;

	public function __construct() {
		assert('non_class_scope();');
		var_dump($this->prop);
	}
}

new test;

?>
