<?php

trait foo {	
	public function __toString() {
		return '123';
	}
	
	public function __get($x) {
		var_dump($x);
	}
	
	public function __set($attr, $val) {
		var_dump($attr .'==='. $val);
	}
	
	public function __clone() {
		var_dump(__FUNCTION__);
	}
}

class bar {
	use foo;
}

$o = new bar;
echo $o, PHP_EOL;
$o->xyz;
$o->xyz = 2;
clone $o;

?>