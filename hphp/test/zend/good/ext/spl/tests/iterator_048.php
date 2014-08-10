<?php

class MyRecursiveRegexIterator extends RecursiveRegexIterator
{
	function show()
	{
		foreach(new RecursiveIteratorIterator($this) as $k => $v)
		{
			var_dump($k);
			var_dump($v);
		}
	}

	function accept()
	{
		return $this->hasChildren() || parent::accept();
	}
}

$ar = new RecursiveArrayIterator(array('Foo', array('Bar'), 'FooBar', array('Baz'), 'Biz'));
$it = new MyRecursiveRegexIterator($ar, '/Bar/');

$it->show();

?>
===DONE===
<?php exit(0); ?>
