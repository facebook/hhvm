<?php
class test { 
	protected $foo;
	private $bar;
	public $test;

	function foo()
	{
		$this->bar = 'meuh';
		$this->foo = 'lala';
		$this->test = 'test';

		var_dump(http_build_query($this));
	}
}

$obj = new test();
$obj->foo();
var_dump(http_build_query($obj));
?>