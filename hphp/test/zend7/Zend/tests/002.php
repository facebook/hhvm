<?php

function test1() {
	var_dump(func_get_arg(-10));
	var_dump(func_get_arg(0));
	var_dump(func_get_arg(1));
}

function test2($a) {
	var_dump(func_get_arg(0));
	var_dump(func_get_arg(1));
}

function test3($a, $b) {
	var_dump(func_get_arg(0));
	var_dump(func_get_arg(1));
	var_dump(func_get_arg(2));
}

test1();
test1(10);
test2(1);
try {
	test2();
} catch (Throwable $e) {
	echo "Exception: " . $e->getMessage() . "\n";
}
test3(1,2);

call_user_func("test1");
try {
	call_user_func("test3", 1);
} catch (Throwable $e) {
	echo "Exception: " . $e->getMessage() . "\n";
}
call_user_func("test3", 1, 2);

class test {
	static function test1($a) {
		var_dump(func_get_arg(0));
		var_dump(func_get_arg(1));
	}
}

test::test1(1);
var_dump(func_get_arg(1));

echo "Done\n";
?>
