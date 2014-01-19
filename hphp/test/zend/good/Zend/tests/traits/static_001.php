<?php

	trait TestTrait {
		public static function test() {
			return 'Test';
		}
	}

	class A {
		use TestTrait;
	}

	echo A::test();

?>