<?php

function test($ar, $flags)
{
	echo "===$flags===\n";
	$it = new CachingIterator($ar, 0);
	try
	{
		$it->setFlags($flags);
	}
	catch (Exception $e)
	{
		echo 'Exception: ' . $e->getMessage() . "\n";
		var_dump($it->getFlags());
		return;
	}
	var_dump($it->getFlags());
	try
	{
		foreach($it as $v)
		{
			var_dump((string)$it);
		}
	}
	catch (Exception $e)
	{
		echo 'Exception: ' . $e->getMessage() . "\n";
	}
}

class MyItem
{
	function __construct($value)
	{
		$this->value = $value;
	}

	function __toString()
	{
		return (string)$this->value;
	}
}

class MyArrayIterator extends ArrayIterator
{
	function __toString()
	{
		return $this->key() . ':' . $this->current();
	}
}

$ar = new MyArrayIterator(array(1, 2, 3));

test($ar, CachingIterator::CALL_TOSTRING);
test($ar, CachingIterator::TOSTRING_USE_KEY);
test($ar, CachingIterator::TOSTRING_USE_CURRENT);

$ar = new MyArrayIterator(array(new MyItem(1), new MyItem(2), new MyItem(3)));

test($ar, CachingIterator::TOSTRING_USE_INNER);
test($ar, CachingIterator::CALL_TOSTRING | CachingIterator::TOSTRING_USE_KEY);
test($ar, CachingIterator::CALL_TOSTRING | CachingIterator::TOSTRING_USE_CURRENT);
test($ar, CachingIterator::CALL_TOSTRING | CachingIterator::TOSTRING_USE_INNER);
test($ar, CachingIterator::TOSTRING_USE_KEY | CachingIterator::TOSTRING_USE_CURRENT);
test($ar, CachingIterator::TOSTRING_USE_KEY | CachingIterator::TOSTRING_USE_INNER);

echo "===X===\n";
try
{
	$it = new CachingIterator($ar, CachingIterator::CALL_TOSTRING);
	$it->setFlags(0);
}
catch (Exception $e)
{
	echo 'Exception: ' . $e->getMessage() . "\n";
}
try
{
	$it = new CachingIterator($ar, CachingIterator::TOSTRING_USE_INNER);
	$it->setFlags(0);
}
catch (Exception $e)
{
	echo 'Exception: ' . $e->getMessage() . "\n";
}

?>
===DONE===