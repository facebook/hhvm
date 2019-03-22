<?php
class A {
	static function test($v='') {
		print_r(static::class);
	}
}
class B extends A {
}
B::test();
spl_autoload_register('B::test');
new X();
