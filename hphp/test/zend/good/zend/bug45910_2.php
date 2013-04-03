<?php

class foo {
	const AAA = 'x';
	const BBB = 'a';
	const CCC = 'a';
	const DDD = self::AAA;

	private static $foo = array(
		self::BBB	=> 'a',
		self::CCC	=> 'b',
		self::DDD	=>  11
	);
	
	public static function test() {
		self::$foo;
	}
}

foo::test();

print 1;
?>