<?php

error_reporting(E_ALL);

class foo {
	public $x = array(1);
		
	public function &b() {
		return $this->x;
	}
}

$foo = new foo;

$a = 'b';
var_dump($foo->$a()[0]);

$h = &$foo->$a();
$h[] = 2;
var_dump($foo->$a());

?>