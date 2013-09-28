<?php

class MyRecursiveArrayIterator extends RecursiveArrayIterator
{
	function valid()
	{
		if (!parent::valid())
		{
			echo __METHOD__ . " = false\n";
			return false;
		}
		else
		{
			return true;
		}
	}

	function getChildren()
	{
		echo __METHOD__ . "\n";
		return parent::getChildren();
	}
}

class RecursiveArrayIteratorIterator extends RecursiveIteratorIterator
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

	function beginChildren()
	{
		echo __METHOD__ . "(".$this->getDepth().")\n";
	}

	function endChildren()
	{
		echo __METHOD__ . "(".$this->getDepth().")\n";
	}
}

foreach(new RecursiveArrayIteratorIterator(new MyRecursiveArrayIterator(array("a", array("ba", array("bba", "bbb"), array(array("bcaa"))), array("ca"), "d"))) as $k=>$v)
{
	echo "$k=>$v\n";
}
?>
===DONE===
<?php exit(0); ?>