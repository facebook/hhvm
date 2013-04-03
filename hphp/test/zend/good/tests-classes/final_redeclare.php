<?php

class pass {
	final function show() {
		echo "Call to function pass::show()\n";
	}
}

$t = new pass();

class fail extends pass {
	function show() {
		echo "Call to function fail::show()\n";
	}
}

echo "Done\n"; // Shouldn't be displayed
?>