<?php

function f() {
	echo "in f()\n";
	return "name";
}

function g() {
	echo "in g()\n";
	return "assigned value";
}


echo "\n\nOrder with array assignment:\n";
$a = array();
$a[f()] = g();
var_dump($a);


class C {
	public static $name = "original";
	public static $a = array();
	public static $string = "hello";
}

echo "\n\nOrder with static array property assignment:\n";
C::$a[f()] = g();
var_dump(C::$a);

echo "\n\nOrder with indexed string assignment:\n";
$string = "hello";
function getOffset() {
	echo "in getOffset()\n";
	return 0;
}
function newChar() {
	echo "in newChar()\n";
	return 'j';
}
$string[getOffset()] = newChar();
var_dump($string);

echo "\n\nOrder with static string property assignment:\n";
C::$string[getOffset()] = newChar();
var_dump(C::$string);
