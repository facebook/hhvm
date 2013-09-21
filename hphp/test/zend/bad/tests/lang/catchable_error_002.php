<?php
	class Foo {
	}

	function blah (Foo $a)
	{
	}

	function error()
	{
		$a = func_get_args();
		var_dump($a);
	}

	set_error_handler('error');

	blah (new StdClass);
	echo "ALIVE!\n";
?>