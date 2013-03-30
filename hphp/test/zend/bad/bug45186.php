<?php

class bar  {
	public function __call($a, $b) {
		print "__call:\n";
		var_dump($a);
	}
	static public function __callstatic($a, $b) {
		print "__callstatic:\n";
		var_dump($a);
	}
	public function test() {
		self::ABC();
		bar::ABC();
		call_user_func(array('BAR', 'xyz'));
		call_user_func('BAR::www');
		call_user_func(array('self', 'y'));
		call_user_func('self::y');
	}
	static function x() { 
		print "ok\n";
	}
}

$x = new bar;

$x->test();

call_user_func(array('BAR','x'));
call_user_func('BAR::www');
call_user_func('self::y');

?>