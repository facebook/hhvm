<?php

class foo {
	public function test(&$x) {
		static $lambda;
		$lambda = function &() use (&$x) { 
			return $x = $x * $x;
		};
		return $lambda();
	}
}

$test = new foo;

$y = 2;
var_dump($test->test($y));
var_dump($x = $test->test($y));
var_dump($y, $x);

?>