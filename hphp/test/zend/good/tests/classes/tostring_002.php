<?php

class Test
{
	function __toString()
	{
		return "Hello\n";
	}
	
	function __destruct()
	{
		echo $this;
	}
}

$o = new Test;
$o = NULL;

$o = new Test;

?>
====DONE====
