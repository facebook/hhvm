<?php
class A
{
	protected $bar = array('baz');

	function bar()
	{
		array_pop($this->bar);
		$vars = get_object_vars($this);
		$this->bar[] = array('buz');
		print_r($vars);
	}

	function foo() {
		array_pop($this->bar);
		$dummy = &$this->bar;
		$vars = get_object_vars($this);
		$this->bar[] = array('buz');
		print_r($vars);
	}
}

(new A())->bar();
(new A())->foo();
?>
