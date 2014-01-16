<?php 

class foo {
	static $bar = array();
	
	public function __set($a, $b) {
		self::$bar[] = $b;
	}
	
	public function __get($a) {
		/* last */
		return self::$bar[count(self::$bar)-1];
	}
}

function test() {
	return new foo;
}

$a = test()->bar = 1;
var_dump($a, count(foo::$bar), test()->whatever);

print "\n";

$a = test()->bar = NULL;
var_dump($a, count(foo::$bar), test()->whatever);

print "\n";

$a = test()->bar = test();
var_dump($a, count(foo::$bar), test()->whatever);

print "\n";

?>