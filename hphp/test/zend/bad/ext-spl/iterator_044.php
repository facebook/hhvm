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

	function test($ar)
	{
		foreach($ar as $k => $v)
		{
			echo "===$k===\n";
			var_dump($v);
			var_dump($this->offsetExists($v));
			var_dump($this->offsetGet($v));
		}
	}
}

$it = new MyCachingIterator(new ArrayIterator(array(0, 'foo'=>1, 2, 'bar'=>3, 4)));

try
{
	var_dump($it->offsetExists(0));
}
catch(Exception $e)
{
	echo "Exception: " . $e->getMessage() . "\n";
}

try
{
	var_dump($it->offsetGet(0));
}
catch(Exception $e)
{
	echo "Exception: " . $e->getMessage() . "\n";
}

$it = new MyCachingIterator(new ArrayIterator(array(0, 'foo'=>1, 2, 'bar'=>3, 4)), CachingIterator::FULL_CACHE);

var_dump($it->offsetExists());
var_dump($it->offsetGet());

$checks = array(0, new stdClass, new MyFoo, NULL, 2, 'foo', 3);

$it->test($checks);

echo "===FILL===\n";

foreach($it as $v); // read all into cache

$it->test($checks);

?>
===DONE===
<?php exit(0); ?>