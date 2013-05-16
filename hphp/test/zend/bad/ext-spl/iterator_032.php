<?php

$it = new LimitIterator(new ArrayIterator(array(1,2,3,4)), 1, 2);

foreach($it as $k=>$v)
{
	echo "$k=>$v\n";
	var_dump($it->getPosition());
}

try
{
	$it->seek(0);
}
catch(OutOfBoundsException $e)
{
	echo $e->getMessage() . "\n";
}

$it->seek(2);
var_dump($it->current());

try
{
	$it->seek(3);
}
catch(OutOfBoundsException $e)
{
	echo $e->getMessage() . "\n";
}

$it->next();
var_dump($it->valid());

?>
===DONE===
<?php exit(0); ?>