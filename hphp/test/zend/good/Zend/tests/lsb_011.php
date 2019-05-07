<?php

class Test1 {
	static function ok() {
		echo "bug";
	}
	static function test() {
		static::ok();
	}
}

class Test2 extends Test1 {
	static function ok() {
		echo "ok";
	}
}
Test2::test();
