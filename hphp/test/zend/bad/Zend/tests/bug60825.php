<?php
class test {
	public static $x;
	public function __toString() {
		self::$x = $this;
		return __FILE__;
	}
}
$a = new test;
require_once $a;
debug_zval_dump(test::$x);
?>