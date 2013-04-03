<?php

class Test
{
	function __toString()
	{
		throw new Exception("Damn!");
		return "Hello\n";
	}
	
	function __destruct()
	{
		echo $this;
	}
}

try
{
	$o = new Test;
	$o = NULL;
}
catch(Exception $e)
{
	var_dump($e->getMessage());
}

?>
====DONE====