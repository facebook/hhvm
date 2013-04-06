<?php

error_reporting(E_ALL);

class foo {
	public $arr;
	
	public function &a() {
		return $this->arr;
	}
}

$foo = new foo;

$h = &$foo->a();
$h[] = 1;
$h[] = $foo;
var_dump($foo->a()[1]->arr[1]->a()[1]->arr[1]->arr[0]);
var_dump($foo->a()[1]);
var_dump($foo->a()[1]->arr[1]);

?>