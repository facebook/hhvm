<?php

function walk($key, $value) { 
	var_dump($value, $key); 
}

class test {
	private $var_pri = "test_private";
	protected $var_pro = "test_protected";
	public $var_pub = "test_public";
}

$stdclass = new stdclass;
$stdclass->foo = "foo";
$stdclass->bar = "bar";
array_walk_recursive($stdclass, "walk");

$t = new test;
array_walk_recursive($t, "walk");

$var = array();
array_walk_recursive($var, "walk");
$var = "";
array_walk_recursive($var, "walk");

echo "Done\n";
?>