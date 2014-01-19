<?php

final class base {
	function show() {
		echo "base\n";
	}
}

$t = new base();

class derived extends base {
}

echo "Done\n"; // shouldn't be displayed
?>