<?php

function test1() {
	var_dump(func_num_args());
}

function test2($a) {
	var_dump(func_num_args());
}

function test3($a, $b) {
	var_dump(func_num_args());
}

test1();
test2(1);
try { test2(); } catch (Exception $e) { var_dump($e->getMessage()); }
test3(1,2);

call_user_func("test1");
try { call_user_func("test3", 1); } catch (Exception $e) { var_dump($e->getMessage()); }
call_user_func("test3", 1, 2);

class test {
	static function test1($a) {
		var_dump(func_num_args());
	}
}

test::test1(1);
var_dump(func_num_args());

echo "Done\n";
?>
