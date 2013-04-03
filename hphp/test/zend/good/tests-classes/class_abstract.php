<?php

abstract class base {
	function show() {
		echo "base\n";
	}
}

class derived extends base {
}

$t = new derived();
$t->show();

$t = new base();
$t->show();

echo "Done\n"; // shouldn't be displayed
?>