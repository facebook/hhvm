<?php

class pass {
	static function show() {
		echo "Call to function pass::show()\n";
	}
}

class fail extends pass {
	function show() {
		echo "Call to function fail::show()\n";
	}
}

pass::show();
fail::show();

echo "Done\n"; // shouldn't be displayed
?>                                                            