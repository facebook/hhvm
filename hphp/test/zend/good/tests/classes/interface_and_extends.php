<?php

interface Test
{
	function show();
}

class Tester extends Test
{
	function show() {
		echo __METHOD__ . "\n";
	}
}

$o = new Tester;
$o->show();

?>
===DONE===