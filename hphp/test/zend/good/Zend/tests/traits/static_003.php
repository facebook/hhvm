<?php

	trait TestTrait {
		public static function test() {
			return static::$test;
		}
	}

	class A {
		use TestTrait;
		protected static $test = "Test A";
	}

	class B extends A {
		protected static $test = "Test B";
	}

	echo B::test();

?>