<?php

class MyFoo
{
	function __toString()
	{
		return 'foo';
	}
}

class MyCachingIterator extends CachingIterator
{
	function __construct(Iterator $it, $flags = 0)
	{
		parent::__construct($it, $flags);
	}

	function testSet($ar)
	{
		echo __METHOD__ . "()\n";
		foreach($ar as $k => $v)
		{
			echo "set($k,$v)\n";
			$this->offsetSet($k, $v);
		}
	}
	
	function testUnset($ar)
	{
		echo __METHOD__ . "()\n";
		foreach($ar as $k => $v)
		{
			echo "unset($v)\n";
			$this->offsetUnset($v);
		}
	}
	
	function fill()
	{
		echo __METHOD__ . "()\n";
		foreach($this as $v) ;
	}

	function show()
	{
		echo __METHOD__ . "()\n";
		var_dump($this->getCache());
	}
}

$it = new MyCachingIterator(new ArrayIterator(array(0, 'foo'=>1, 2, 'bar'=>3, 4)));

try
{
	var_dump($it->offsetSet(0, 0));
}
catch(Exception $e)
{
	echo "Exception: " . $e->getMessage() . "\n";
}

try
{
	var_dump($it->offsetUnset(0));
}
catch(Exception $e)
{
	echo "Exception: " . $e->getMessage() . "\n";
}

$it = new MyCachingIterator(new ArrayIterator(array(0, 1, 2, 3)), CachingIterator::FULL_CACHE);

var_dump($it->offsetSet());
var_dump($it->offsetSet(0));
var_dump($it->offsetUnset());

$checks = array(0 => 25, 1 => 42, 3 => 'FooBar');
$unsets = array(0, 2);

$it->testSet($checks);
$it->show();
$it->testUnset($unsets);
$it->show();
$it->fill();
$it->show();
$it->testSet($checks);
$it->show();
$it->testUnset($unsets);
$it->show();

?>
===DONE===
<?php exit(0); ?>
