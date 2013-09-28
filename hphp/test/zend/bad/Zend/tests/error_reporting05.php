<?php

error_reporting(E_ALL);
	
class test {
	function __get($name) {
		return $undef_name;
	}
	function __set($name, $value) {
		return $undef_value;
	}
}

$test = new test;

$test->abc = 123;
echo $test->bcd;

@$test->qwe = 123;
echo @$test->wer;

var_dump(error_reporting());

echo "Done\n";
?>