<?php

class foo {
	public $x = 1;
}

class bar {
	public $y = 'foo';
}

$x = 'bar';

$bar = new bar;

var_dump((new bar)->y);     // foo
var_dump((new $x)->y);      // foo
var_dump((new $bar->y)->x); // 1

?>