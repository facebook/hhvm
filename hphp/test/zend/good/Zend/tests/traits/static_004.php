<?php

	trait TestTrait {
		public static function __callStatic($name, $arguments) {
			return $name;
		}
	}

	class A {
		use TestTrait;
	}

	echo A::Test();

?>