<?php

class ArrayIteratorEx extends ArrayIterator
{
	function rewind()
	{
		echo __METHOD__ . "\n";
		ArrayIterator::rewind();
	}

	function valid()
	{
		echo __METHOD__ . "\n";
		return ArrayIterator::valid();
	}

	function key()
	{
		echo __METHOD__ . "\n";
		return ArrayIterator::key();
	}

	function current()
	{
		echo __METHOD__ . "\n";
		return ArrayIterator::current();
	}

	function next()
	{
		echo __METHOD__ . "\n";
		return ArrayIterator::next();
	}
}

$ar = new ArrayIteratorEx(array(1,2));
foreach($ar as $k => $v)
{
	var_dump($k);
	var_dump($v);
}

?>
===DONE===
<?php exit(0); ?>