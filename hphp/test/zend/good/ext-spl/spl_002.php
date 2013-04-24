<?php

class Test implements Countable
{
	function count()
	{
		return 4;
	}
};

$a = new Test;

var_dump(count($a));

?>
===DONE===