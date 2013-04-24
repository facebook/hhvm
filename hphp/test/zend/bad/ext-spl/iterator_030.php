<?php

$it = new EmptyIterator;

var_dump($it->valid());
$it->rewind();
var_dump($it->valid());
$it->next();
var_dump($it->valid());

try
{
	var_dump($it->key());
}
catch(BadMethodCallException $e)
{
	echo $e->getMessage() . "\n";
}

try
{
	var_dump($it->current());
}
catch(BadMethodCallException $e)
{
	echo $e->getMessage() . "\n";
}

var_dump($it->valid());

?>
===DONE===
<?php exit(0); ?>