<?hh

class Test1 {
	static function test() {
		var_dump(is_callable(static::class."::ok"));
		var_dump(is_callable(array(static::class,"ok")));
	}
}

class Test2 extends Test1 {
	static function ok() {
	}
}
Test1::test();
Test2::test();
