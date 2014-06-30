<?php

$it = new ArrayIterator(range(0,10));
var_dump($it->count());
$it->seek(5);
var_dump($it->current());
$it->seek(4);
var_dump($it->current());
try
{
	$it->seek(-1);
	var_dump($it->current());
}
catch(Exception $e)
{
	echo $e->getMessage() . "\n";
}

try
{
	$it->seek(12);
	var_dump($it->current());
}
catch(Exception $e)
{
	echo $e->getMessage() . "\n";
}

$pos = 0;
foreach($it as $v)
{
	$it->seek($pos++);
	var_dump($v);
}

?>
===DONE===
<?php exit(0); ?>