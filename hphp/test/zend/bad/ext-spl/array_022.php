==ArrayObject===
<?php

class MyArrayObject extends ArrayObject
{
	public function __construct()
	{
		parent::__construct($this);
		$this['bar'] = 'baz';
	}
}

$a = new MyArrayObject;

$b = clone $a;
$b['baz'] = 'Foo';

var_dump($a);
var_dump($b);

?>
==ArrayIterator===
<?php

class MyArrayIterator extends ArrayIterator
{
	public function __construct()
	{
		parent::__construct($this);
		$this['bar'] = 'baz';
	}
}

$a = new MyArrayIterator;

$b = clone $a;
$b['baz'] = 'Foo';

var_dump($a);
var_dump($b);

?>
===DONE===