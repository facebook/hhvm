<?php

class A
{
	function __destruct()
	{
		$myArray = array();

		for($i = 1; $i <= 3000; $i++) {
			if(!isset($myArray[$i]))
				$myArray[$i] = array();
			$ref = & $myArray[$i];
			$ref[] = new stdClass();
		}
	}
}

$a = new A();

echo "Done\n";
?>