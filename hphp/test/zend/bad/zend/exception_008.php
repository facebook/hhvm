<?php

class TestFirst
{
	function __destruct() {
		throw new Exception("First");
	}
}

class TestSecond
{
	function __destruct() {
		throw new Exception("Second");		
	}
}

$ar = array(new TestFirst, new TestSecond);

unset($ar);

?>
===DONE===