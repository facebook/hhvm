<?php

class bar {
}

class foo { 
	public function test() {
		class_alias('bar', 'static');
		return new static;
	}
}

$a = new foo;
var_dump($a->test());

?>