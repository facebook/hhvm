<?php
function test_1() {
	static $v = 0;
	++$v;
	echo "Outer function increments \$v to $v\n";
	$f = function() use($v) {
		echo "Inner function reckons \$v is $v\n";
	};
	return $f;
}

$f = test_1(); $f();
$f = test_1(); $f();

function test_2() {
	static $v = 0;
	$f = function() use($v) {
		echo "Inner function reckons \$v is $v\n";
	};
	++$v;
	echo "Outer function increments \$v to $v\n";
	return $f;
}

$f = test_2(); $f();
$f = test_2(); $f();

function test_3() {
	static $v = "";
	$v .= 'b';
	echo "Outer function catenates 'b' onto \$v to give $v\n";
	$f = function() use($v) {
		echo "Inner function reckons \$v is $v\n";
	};
	$v .= 'a';
	echo "Outer function catenates 'a' onto \$v to give $v\n";
	return $f;
}
$f = test_3(); $f();
$f = test_3(); $f();