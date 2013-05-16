<?php

foreach(new NoRewindIterator(new ArrayIterator(array('Hello'=>0, 'World'=>1))) as $k => $v)
{
	var_dump($v);
	var_dump($k);
}

?>
===DONE===