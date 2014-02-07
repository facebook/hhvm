<?php
class Test extends mysqli
{
	public $test = array();

	function foo()
	{
		$ar_test = array("foo", "bar");
		$this->test = &$ar_test;
	}
}

$my_test = new Test;
$my_test->foo();
var_dump($my_test->test);
?>