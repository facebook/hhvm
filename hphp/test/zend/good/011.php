<?php

class foo {
	public $pp1 = 1;
	private $pp2 = 2;
	protected $pp3 = 3;

	function bar() {
		var_dump(property_exists("foo","pp1"));
		var_dump(property_exists("foo","pp2"));
		var_dump(property_exists("foo","pp3"));
	}
}

class bar extends foo {
	function test() {
		var_dump(property_exists("foo","pp1"));
		var_dump(property_exists("foo","pp2"));
		var_dump(property_exists("foo","pp3"));
	}
}

var_dump(property_exists());
var_dump(property_exists(""));
var_dump(property_exists("foo","pp1"));
var_dump(property_exists("foo","pp2"));
var_dump(property_exists("foo","pp3"));
var_dump(property_exists("foo","nonexistent"));
var_dump(property_exists("fo","nonexistent"));
var_dump(property_exists("foo",""));
var_dump(property_exists("","test"));
var_dump(property_exists("",""));

$foo = new foo;

var_dump(property_exists($foo,"pp1"));
var_dump(property_exists($foo,"pp2"));
var_dump(property_exists($foo,"pp3"));
var_dump(property_exists($foo,"nonexistent"));
var_dump(property_exists($foo,""));
var_dump(property_exists(array(),"test"));
var_dump(property_exists(1,"test"));
var_dump(property_exists(true,"test"));

$foo->bar();

$bar = new bar;
$bar->test();

echo "Done\n";
?>