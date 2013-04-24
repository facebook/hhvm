<?php

$it = new ArrayIterator(range(0,3));

foreach(new IteratorIterator($it) as $v)
{
	var_dump($v);
}

$it = new ArrayObject(range(0,3));

foreach(new IteratorIterator($it) as $v)
{
	var_dump($v);
}

?>
===DONE===
<?php exit(0); ?>