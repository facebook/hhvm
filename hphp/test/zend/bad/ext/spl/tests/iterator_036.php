<?php

function test($it)
{
	foreach($it as $v)
	{
		var_dump((string)$it);
	}
}

$ar = new ArrayIterator(array(1, 2, 3));

test(new CachingIterator($ar, 0));

?>
===DONE===