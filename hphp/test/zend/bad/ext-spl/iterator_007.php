<?php

class ArrayIteratorEx extends ArrayIterator
{
	function rewind()
	{
		echo __METHOD__ . "\n";
		parent::rewind();
	}
	function valid()
	{
		echo __METHOD__ . "\n";
		return parent::valid();
	}
	function current()
	{
		echo __METHOD__ . "\n";
		return parent::current();
	}
	function key()
	{
		echo __METHOD__ . "\n";
		return parent::key();
	}
	function next()
	{
		echo __METHOD__ . "\n";
		parent::next();
	}
}

class NoRewindIteratorEx extends NoRewindIterator
{
	function rewind()
	{
		echo __METHOD__ . "\n";
		parent::rewind();
	}
	function valid()
	{
		echo __METHOD__ . "\n";
		return parent::valid();
	}
	function current()
	{
		echo __METHOD__ . "\n";
		return parent::current();
	}
	function key()
	{
		echo __METHOD__ . "\n";
		return parent::key();
	}
	function next()
	{
		echo __METHOD__ . "\n";
		parent::next();
	}
}

$it = new NoRewindIteratorEx(new ArrayIteratorEx(range(0,3)));

echo "===0===\n";
foreach ($it->getInnerIterator() as $v) {
	var_dump($v);
}

echo "===1===\n";
foreach ($it as $v) {
	var_dump($v);
}

$pos =0;

$it = new NoRewindIteratorEx(new ArrayIteratorEx(range(0,3)));

echo "===2===\n";
foreach ($it as $v) {
	var_dump($v);
	if ($pos++ > 1) {
		break;
	}
}

echo "===3===\n";
foreach ($it as $v) {
	var_dump($v);
}

echo "===4===\n";
foreach ($it as $v) {
	var_dump($v);
}
?>
===DONE===
<?php exit(0); ?>